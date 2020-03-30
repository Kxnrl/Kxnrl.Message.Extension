#ifndef __KMESSAGE_WEBSOCKET_INCLUDE__
#define __KMESSAGE_WEBSOCKET_INCLUDE__

#include "smsdk_ext.h"

#include <memory>
#include <string>
#include <mutex>
#include <queue>
#include <condition_variable>

#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

template<typename S>
class WSClient
{
public:
    WSClient(std::string host, std::string port, std::string path, std::shared_ptr<boost::asio::io_context> netc, std::shared_ptr<boost::asio::io_context> gamec, float interval);

    void Start();
    void Send(std::string data);
    void Stop();

    bool IsOpen();

public:
    void fail(boost::system::error_code ec, const std::string& what);
    void fail_send(boost::system::error_code ec, const std::string& what, const std::string& data);
    void on_start();
    void on_resolve(boost::system::error_code ec, boost::asio::ip::tcp::tcp::resolver::results_type results);
    void on_run_stream(boost::asio::ip::tcp::resolver::results_type results);
    void on_connect(boost::system::error_code ec);
    void on_handshake(boost::system::error_code ec);
    void on_run();
    void on_websocket_handshake(boost::system::error_code ec);
    void do_write();
    void on_write(boost::system::error_code ec, std::size_t bytes_transferred, const std::string &data);
    void do_read();
    void on_read(boost::system::error_code ec, std::size_t bytes_transferred);
    void on_ping_start();
    void on_ping();

private:
    std::string m_Host;
    std::string m_Port;
    std::string m_Path;
    float m_PingInterval;
    boost::beast::flat_buffer buffer;

private:
    std::shared_ptr<boost::asio::io_context> m_IoContext;
    std::shared_ptr<boost::asio::io_context> m_GameContext;
    std::unique_ptr<boost::asio::ip::tcp::resolver> m_Resolver;
    std::unique_ptr<boost::beast::websocket::stream<S>> m_Ws;
    std::unique_ptr<boost::asio::ssl::context> m_SslContext;
    std::unique_ptr<boost::asio::steady_timer> m_PingTimer;

    std::queue<std::string> m_Queue;
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCV;
};

extern sp_nativeinfo_t WebSocketNatives[];

#define WEBSOCKET_SERVER_ADDRESS "ws://ws.kxnrl.com/"

#endif // ! __KMESSAGE_WEBSOCKET_INCLUDE__