#include <chrono>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include "extension.h"
#include "websocket.h"
#include "message.h"

namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;


extern void reportError(boost::system::system_error err);
extern void pushBuffer(beast::flat_buffer buffer);

template<typename S>
WSClient<S>::WSClient(std::string host, std::string port, std::string path, boost::asio::io_context& netc, boost::asio::io_context& gamec, float interval)
    : m_Host(std::move(host)), m_Port(std::move(port)), m_Path(std::move(path)), m_IoContext(&netc), m_GameContext(&gamec), m_PingInterval(interval)
{
    m_Ws = std::make_unique<boost::beast::websocket::stream<S>>(netc);
    m_PingTimer = std::make_unique<boost::asio::steady_timer>(netc);
}

template<>
WSClient<beast::ssl_stream<beast::tcp_stream>>::WSClient(std::string host, std::string port, std::string path, boost::asio::io_context& netc, boost::asio::io_context& gamec, float interval)
    : m_Host(std::move(host)), m_Port(std::move(port)), m_Path(std::move(path)), m_IoContext(&netc), m_GameContext(&gamec), m_PingInterval(interval)
{
    m_SslContext = std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client);
    m_Ws = std::make_unique<boost::beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(netc, *m_SslContext);
    m_PingTimer = std::make_unique<boost::asio::steady_timer>(netc);
}

template<typename S>
void WSClient<S>::Start()
{
    boost::asio::co_spawn(*m_IoContext, boost::bind(&WSClient::co_entry, this), boost::asio::detached);
    boost::asio::co_spawn(*m_IoContext, boost::bind(&WSClient::co_ping, this), boost::asio::detached);
}

template<typename S>
void WSClient<S>::Send(std::vector<uint8_t> data)
{
    m_Ws->async_write(boost::asio::buffer(data), [](boost::system::error_code const&, std::size_t) {
    });
}

template<typename S>
void WSClient<S>::Stop()
{
    beast::get_lowest_layer(*m_Ws).cancel();
    m_PingTimer->cancel();
}

template<typename S>
bool WSClient<S>::IsOpen()
{
    return m_Ws->is_open();
}

template<typename S>
boost::asio::awaitable<void> WSClient<S>::co_entry()
{
    try {
        tcp::resolver resolver(*m_IoContext);
        auto const results = co_await resolver.async_resolve(m_Host, m_Port, boost::asio::use_awaitable);
        co_await co_run_stream(results);
    }
    catch (boost::system::system_error& err)
    {
        boost::asio::dispatch(*m_GameContext, boost::bind(reportError, err));
    }
}

template<typename S>
boost::asio::awaitable<void> WSClient<S>::co_run()
{
    m_Ws->set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
    m_Ws->set_option(beast::websocket::stream_base::decorator(
        [](beast::websocket::request_type& req)
        {
            req.set(beast::http::field::user_agent,
                "Kxnrl.Message Extension");
        }));

    co_await m_Ws->async_handshake(m_Host, m_Path, boost::asio::use_awaitable);

    while (1) {
        beast::flat_buffer buffer;
        co_await m_Ws->async_read(buffer, boost::asio::use_awaitable);
        boost::asio::dispatch(*m_GameContext, boost::bind(pushBuffer, buffer));
    }
}

template<typename S>
boost::asio::awaitable<void> WSClient<S>::co_ping()
{
    try {
        while (1) {
            m_PingTimer->expires_after(std::chrono::milliseconds((int64_t)(m_PingInterval) * 1000));
            co_await m_PingTimer->async_wait(boost::asio::use_awaitable);
            
            auto message = std::make_unique<KMessage>(Message_Type::PingPong);
            message->WriteInt64("time", static_cast<int64_t>(std::time(NULL)));
            message->WriteString("ping", "pong");
            auto pingdata = message->JsonString();

            Send(std::vector<uint8_t>((uint8_t*)pingdata.data(), (uint8_t*)(pingdata.data()+pingdata.size())));
        }
    }
    catch (std::exception&)
    {
    }
}

template<>
boost::asio::awaitable<void> WSClient<beast::ssl_stream<beast::tcp_stream>>::co_run_stream(tcp::resolver::results_type results)
{
    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(30));
    co_await beast::get_lowest_layer(*m_Ws).async_connect(results, boost::asio::use_awaitable);

    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(30));
    co_await m_Ws->next_layer().async_handshake(boost::asio::ssl::stream_base::handshake_type::client, boost::asio::use_awaitable);

    beast::get_lowest_layer(*m_Ws).expires_never();
    co_await co_run();
}

template<>
boost::asio::awaitable<void> WSClient<beast::tcp_stream>::co_run_stream(tcp::resolver::results_type results)
{
    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(30));
    co_await beast::get_lowest_layer(*m_Ws).async_connect(results, boost::asio::use_awaitable);

    beast::get_lowest_layer(*m_Ws).expires_never();
    co_await co_run();
}

template class WSClient<beast::ssl_stream<beast::tcp_stream>>;
template class WSClient<beast::tcp_stream>;
