#ifndef __KMESSAGE_WEBSOCKET_INCLUDE__
#define __KMESSAGE_WEBSOCKET_INCLUDE__

#include "smsdk_ext.h"

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

template<typename S>
class WSClient
{
public:
    WSClient(std::string host, std::string port, std::string path, boost::asio::io_context& netc, boost::asio::io_context& gamec, float interval);

    void Start();
    void Send(std::string data);
    void Stop();

    bool IsOpen();

public:
    boost::asio::awaitable<void> co_entry();
    boost::asio::awaitable<void> co_run_stream(boost::asio::ip::tcp::resolver::results_type results);
    boost::asio::awaitable<void> co_run();
    boost::asio::awaitable<void> co_ping();

private:
    std::string m_Host;
    std::string m_Port;
    std::string m_Path;
    float m_PingInterval;

private:
    boost::asio::io_context* m_IoContext;
    boost::asio::io_context* m_GameContext;
    std::unique_ptr<boost::beast::websocket::stream<S>> m_Ws;
    std::unique_ptr<boost::asio::ssl::context> m_SslContext;
    std::unique_ptr<boost::asio::steady_timer> m_PingTimer;
};

extern sp_nativeinfo_t WebSocketNatives[];

#define WEBSOCKET_SERVER_ADDRESS "ws://ws.kxnrl.com/"

#endif // ! __KMESSAGE_WEBSOCKET_INCLUDE__