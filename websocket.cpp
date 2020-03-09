#include <chrono>
#include <boost/bind.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include "websocket.h"
#include "message.h"

namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;


extern void reportError(boost::system::system_error err);
extern void pushBuffer(beast::flat_buffer buffer);

template<typename S>
WSClient<S>::WSClient(std::string host, std::string port, std::string path, std::shared_ptr<boost::asio::io_context> netc, std::shared_ptr<boost::asio::io_context> gamec, float interval)
    : m_Host(std::move(host)), m_Port(std::move(port)), m_Path(std::move(path)), m_PingInterval(interval), m_IoContext(netc), m_GameContext(gamec)
{
    m_Ws = std::make_unique<boost::beast::websocket::stream<S>>(*netc);
    m_PingTimer = std::make_unique<boost::asio::steady_timer>(*netc);
}

template<>
WSClient<beast::ssl_stream<beast::tcp_stream>>::WSClient(std::string host, std::string port, std::string path, std::shared_ptr<boost::asio::io_context> netc, std::shared_ptr<boost::asio::io_context> gamec, float interval)
    : m_Host(std::move(host)), m_Port(std::move(port)), m_Path(std::move(path)), m_PingInterval(interval), m_IoContext(netc), m_GameContext(gamec)
{
    m_SslContext = std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client);
    m_Ws = std::make_unique<boost::beast::websocket::stream<beast::ssl_stream<beast::tcp_stream>>>(*netc, *m_SslContext);
    m_PingTimer = std::make_unique<boost::asio::steady_timer>(*netc);
}

template<typename S>
void WSClient<S>::Start()
{
    boost::asio::spawn(*m_IoContext, boost::bind(&WSClient<S>::co_entry, this, _1));
}

template<typename S>
void WSClient<S>::Send(std::string data)
{
    extern void PushSendQueue(const std::string &data);

    auto log_and_push_queue = [](const std::string &data, const char *message) {
        smutils->LogMessage(myself, "Unable to send content with error: %s, pushing to send queue... [%s]", message, data.c_str());
        PushSendQueue(data);
    };

    if (!IsOpen()) {
        log_and_push_queue(data, "Not opened");
    }
    m_Ws->async_write(boost::asio::buffer(data), [data, log_and_push_queue](boost::system::error_code const& ec, std::size_t) {
        if (ec) {
            log_and_push_queue(data, ec.message().c_str());
        }
    });
}

template<typename S>
void WSClient<S>::Stop()
{
    if (IsOpen()) {
        boost::system::error_code ec;
        m_Ws->close(beast::websocket::close_reason(beast::websocket::close_code::normal), ec);
    }

    try {
        m_PingTimer->cancel();
    }
    catch (std::exception &)
    {
    }
}

template<typename S>
bool WSClient<S>::IsOpen()
{
    return m_Ws->is_open();
}

template<typename S>
void WSClient<S>::co_entry(boost::asio::yield_context yield)
{
    try {
        tcp::resolver resolver(*m_IoContext);
        auto const results = resolver.async_resolve(m_Host, m_Port, yield);
        if (results.size() <= 0) {
            throw boost::system::system_error(boost::asio::error::netdb_errors::host_not_found);
        }
        co_run_stream(results, yield);
    }
    catch (boost::system::system_error& err)
    {
        boost::asio::dispatch(*m_GameContext, boost::bind(reportError, err));
    }
}

template<typename S>
void WSClient<S>::co_run(boost::asio::yield_context yield)
{
    beast::get_lowest_layer(*m_Ws).expires_never();

    m_Ws->set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
    m_Ws->set_option(beast::websocket::stream_base::decorator(
        [](beast::websocket::request_type& req)
        {
            req.set(beast::http::field::user_agent,
                "Kxnrl.Message.Extension");
        }));

    m_Ws->async_handshake(m_Host, m_Path, yield);

    smutils->LogMessage(myself, "Socket connected to ws%s://%s:%s", m_SslContext == nullptr ? "" : "s", m_Host.c_str(), m_Port.c_str());

    boost::asio::spawn(*m_IoContext, boost::bind(&WSClient<S>::co_ping, this, _1));

    while (1) {
        beast::flat_buffer buffer;
        m_Ws->async_read(buffer, yield);
        boost::asio::dispatch(*m_GameContext, boost::bind(pushBuffer, buffer));
    }
}

template<typename S>
void WSClient<S>::co_ping(boost::asio::yield_context yield)
{
    try {
        while (1) {
            m_PingTimer->expires_after(std::chrono::milliseconds((int64_t)(m_PingInterval) * 1000));
            m_PingTimer->async_wait(yield);
            
            auto message = std::make_unique<KMessage>(Message_Type::PingPong);
            message->WriteInt64("time", static_cast<int64_t>(std::time(NULL)));
            message->WriteString("ping", "pong");
            auto pingdata = message->JsonString();

            Send(pingdata);
        }
    }
    catch (std::exception&)
    {
    }
}

template<>
void WSClient<beast::ssl_stream<beast::tcp_stream>>::co_run_stream(tcp::resolver::results_type results, boost::asio::yield_context yield)
{
    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(10));
    beast::get_lowest_layer(*m_Ws).async_connect(results, yield);

    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(10));
    m_Ws->next_layer().async_handshake(boost::asio::ssl::stream_base::handshake_type::client, yield);

    co_run(yield);
}

template<>
void WSClient<beast::tcp_stream>::co_run_stream(tcp::resolver::results_type results, boost::asio::yield_context yield)
{
    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(30));
    beast::get_lowest_layer(*m_Ws).async_connect(results, yield);

    co_run(yield);
}

template class WSClient<beast::ssl_stream<beast::tcp_stream>>;
template class WSClient<beast::tcp_stream>;
