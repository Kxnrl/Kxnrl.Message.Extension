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
    m_Resolver = std::make_unique<boost::asio::ip::tcp::resolver>(*m_IoContext);
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
    boost::asio::spawn(*m_IoContext, std::bind(&WSClient::on_start, this));
}

static void log_and_push_queue(const std::string& data, const std::string& message)
{
    extern void PushSendQueue(const std::string & data);
    smutils->LogMessage(myself, "Unable to send content with error: %s, pushing to send queue... [%s]", message, data);
    PushSendQueue(data);
}

template<typename S>
void WSClient<S>::Send(std::string data)
{

    if (!IsOpen()) {
        log_and_push_queue(data, "Not opened");
    }

    //do_write(data);
    {
        std::lock_guard<std::mutex> l(m_QueueMutex);
        m_Queue.push(std::move(data));
    }
    m_QueueCV.notify_one();
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
void WSClient<S>::fail(boost::system::error_code ec, const std::string& what) {
    boost::asio::dispatch(*m_GameContext, std::bind(reportError, boost::system::system_error(ec)));
}

template<typename S>
void WSClient<S>::fail_send(boost::system::error_code ec, const std::string& what, const std::string& data) {
    boost::asio::dispatch(*m_GameContext, std::bind(log_and_push_queue, data, what));
}

template<typename S>
void WSClient<S>::on_start()
{
    m_Resolver->async_resolve(m_Host, m_Port, std::bind(&WSClient::on_resolve, this, std::placeholders::_1, std::placeholders::_2));
}

template<typename S>
void WSClient<S>::on_resolve(boost::system::error_code ec, tcp::resolver::results_type results)
{
    if (ec)
        return fail(ec, "on_resolve");

    boost::asio::dispatch(*m_IoContext, std::bind(&WSClient<S>::on_run_stream, this, results));
}

template<>
void WSClient<beast::ssl_stream<beast::tcp_stream>>::on_run_stream(tcp::resolver::results_type results)
{
    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(10));
    beast::get_lowest_layer(*m_Ws).async_connect(results, std::bind(&WSClient::on_connect, this, std::placeholders::_1));
}

template<>
void WSClient<beast::tcp_stream>::on_run_stream(tcp::resolver::results_type results)
{
    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(30));
    beast::get_lowest_layer(*m_Ws).async_connect(results, std::bind(&WSClient::on_connect, this, std::placeholders::_1));
}

template<>
void WSClient<beast::ssl_stream<beast::tcp_stream>>::on_connect(boost::system::error_code ec)
{
    if (ec)
        return fail(ec, "on_connect");

    beast::get_lowest_layer(*m_Ws).expires_after(std::chrono::seconds(10));
    m_Ws->next_layer().async_handshake(boost::asio::ssl::stream_base::handshake_type::client, std::bind(&WSClient::on_handshake, this, std::placeholders::_1));
}

template<>
void WSClient<beast::tcp_stream>::on_connect(boost::system::error_code ec)
{
    if (ec)
        return fail(ec, "on_connect");

    boost::asio::dispatch(*m_IoContext, std::bind(&WSClient::on_run, this));
}

template<>
void WSClient<beast::ssl_stream<beast::tcp_stream>>::on_handshake(boost::system::error_code ec)
{
    if (ec)
        return fail(ec, "on_handshake");

    boost::asio::dispatch(*m_IoContext, std::bind(&WSClient::on_run, this));
}

template<>
void WSClient<beast::tcp_stream>::on_handshake(boost::system::error_code ec) = delete;

template<typename S>
void WSClient<S>::on_run()
{
    beast::get_lowest_layer(*m_Ws).expires_never();

    m_Ws->set_option(beast::websocket::stream_base::timeout::suggested(beast::role_type::client));
    m_Ws->set_option(beast::websocket::stream_base::decorator(
        [](beast::websocket::request_type& req)
        {
            req.set(beast::http::field::user_agent,
                "Kxnrl.Message.Extension");
        }));

    m_Ws->async_handshake(m_Host, m_Path, std::bind(&WSClient::on_websocket_handshake, this, std::placeholders::_1));
}

template<typename S>
void WSClient<S>::on_websocket_handshake(boost::system::error_code ec)
{
    if (ec)
        return fail(ec, "on_websocket_handshake");
    smutils->LogMessage(myself, "Socket connected to ws%s://%s:%s", m_SslContext == nullptr ? "" : "s", m_Host.c_str(), m_Port.c_str());

    boost::asio::dispatch(*m_IoContext, std::bind(&WSClient::on_ping_start, this));
    do_write();
}

template<typename S>
void WSClient<S>::do_write()
{
    bool bSend = false;
    std::string data;
    {
        std::unique_lock<std::mutex> lk(m_QueueMutex);
        m_QueueCV.wait(lk, [this] {return !m_Queue.empty(); });

        bSend = !m_Queue.empty();
        if (bSend)
        {
            data = m_Queue.front();
            m_Queue.pop();
        }
    }

    if (bSend)
    {
        m_Ws->async_write(boost::asio::buffer(data), std::bind(&WSClient::on_write, this, std::placeholders::_1, std::placeholders::_2, data));
    }
    else
    {
        if (std::chrono::steady_clock::now() > m_PingTimer->expiry())
            on_ping_start();
        std::this_thread::yield();
        boost::asio::post(*m_IoContext, std::bind(&WSClient::do_write, this));
    }
}

template<typename S>
void WSClient<S>::on_write(boost::system::error_code ec, std::size_t bytes_transferred, const std::string &data)
{
    if (ec)
        return fail_send(ec, "on_write", data);

    // Clear the buffer
    buffer.consume(buffer.size());

    do_read();
}

template<typename S>
void WSClient<S>::do_read()
{
    m_Ws->async_read(buffer, std::bind(&WSClient::on_read, this, std::placeholders::_1, std::placeholders::_2));
}

template<typename S>
void WSClient<S>::on_read(boost::system::error_code ec, std::size_t bytes_transferred)
{
    // This indicates that the session was closed
    if (ec == boost::beast::websocket::error::closed)
        return;

    if (ec)
        fail(ec, "read");

    boost::asio::dispatch(*m_GameContext, std::bind(pushBuffer, buffer));
    // TODO ? 
    do_write();
}

template<typename S>
void WSClient<S>::on_ping_start()
{
    m_PingTimer->expires_after(std::chrono::milliseconds((int64_t)(m_PingInterval) * 1000));
    m_PingTimer->async_wait(std::bind(&WSClient::on_ping, this));
}

template<typename S>
void WSClient<S>::on_ping()
{
    auto message = std::make_unique<KMessage>(Message_Type::PingPong);
    message->WriteInt64("time", static_cast<int64_t>(std::time(NULL)));
    message->WriteString("ping", "pong");
    auto pingdata = message->JsonString();

    Send(pingdata);
    boost::asio::dispatch(*m_IoContext, std::bind(&WSClient::on_ping_start, this));
}

template class WSClient<beast::ssl_stream<beast::tcp_stream>>;
template class WSClient<beast::tcp_stream>;
