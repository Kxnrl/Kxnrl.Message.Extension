#include "extension.h"
#include "websocket.h"
#include "message.h"

#include <chrono>
#include <stdlib.h>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

// WebSocket

/* Alias namespace */
// websocketpp
namespace ws_lib = websocketpp::lib;
namespace ws_log = websocketpp::log;
namespace opcode = websocketpp::frame::opcode;

typedef websocketpp::client<websocketpp::config::asio_client> client;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::connection_hdl connection_hdl;
typedef client::connection_ptr connection_ptr;
typedef std::chrono::duration<int, std::micro> dur_type;
typedef std::string string;

const sp_nativeinfo_t WebSocketNatives[] =
{
    {"KxnrlMessage_IsConnected", Native_IsConnected},
    {NULL, NULL}
};

ITimer *g_HeartBeat = NULL;

time_t g_LastSent;
time_t g_Interval;

class HeartBeatTimer : public ITimedEvent
{
public:
    ResultType OnTimer(ITimer *pTimer, void *pData)
    {
        time_t now = time(NULL);

        if (now - g_LastSent < g_Interval)
        {
            // recently
            return Pl_Continue;
        }

        static int count = 0;

        KMessage *message = new KMessage(Message_Type::PingPong);
        message->WriteInt64("time", static_cast<int64_t>(time(NULL)));
        message->WriteString("ping", "pong");
        Send(message->JsonString());
        delete message;
        
        static char buffer[128];
        memset(buffer, 0, sizeof(buffer));
        smutils->Format(buffer, sizeof(buffer), "%sHeartBeat: %d\n", THIS_PREFIX, ++count);
        printf_s(buffer);

        return Pl_Continue;
    }

    void OnTimerEnd(ITimer *pTimer, void *pData)
    {
        
    }
} g_HeartBeatTimer;

class WebSocketClient : public IThread
{
    void RunThread(IThreadHandle *pHandle)
    {
        Init(true);
    }

    void OnTerminate(IThreadHandle *pHandle, bool cancel)
    {
        Disconnect();
    }

public:
    bool Available()
    {
        return m_bConnected && !m_bClosing;
    }

    bool Send(string message)
    {
        if (!Available())
        {
            m_bQueue.push(message);
            smutils->LogMessage(myself, "Socket is unavailable now. Push data to queue.");
            return false;
        }

        g_LastSent = time(NULL);
        m_WebSocket.send(m_Connection_hdl, message, opcode::text);
        return true;
    }

    void Shutdown()
    {
        m_bClosing = true;

        if (m_bConnected)
        {
            m_bConnected = false;
            //g_WebSocket.close(g_Connection, opcode::close, "DISCONNECT");
        }
        if (!m_WebSocket.stopped())
        {
            m_WebSocket.stop();
            smutils->LogMessage(myself, "Shutdown socket on exit.");
        }
    }

private:
    client m_WebSocket;
    connection_hdl m_Connection_hdl;
    connection_ptr m_Connection_ptr;
    bool m_bConnected = false;
    bool m_bConnecting = false;
    bool m_bClosing = false;
    std::queue<string> m_bQueue;
    typedef WebSocketClient self;

    /* Init WebSocket */
    void Init(bool init = false)
    {
        if (init)
        {
            // ASIO
            printf_s("%sInit socket asio service...\n", THIS_PREFIX);
            m_WebSocket.init_asio();
        }
        else
        {
            // Reset
            printf_s("%sReset socket asio service...\n", THIS_PREFIX);
            m_WebSocket.reset();
        }

        // Log
        printf_s("%sSet socket log level...\n", THIS_PREFIX);
        m_WebSocket.set_access_channels(ws_log::alevel::none);
        m_WebSocket.set_error_channels(ws_log::elevel::fatal);
        m_WebSocket.clear_access_channels(ws_log::alevel::frame_header);
        m_WebSocket.clear_access_channels(ws_log::alevel::frame_payload);

        // User-Agent
        printf_s("%sSet socket user-agent...\n", THIS_PREFIX);
        m_WebSocket.set_user_agent("Kxnrl.Message Extension");

        // Params
        m_WebSocket.set_close_handshake_timeout(10000);
        m_WebSocket.set_open_handshake_timeout(10000);

        // Event Handlers
        printf_s("%sSet event handler...\n", THIS_PREFIX);
        m_WebSocket.set_open_handler(bind(&self::OnOpen, this, websocketpp::lib::placeholders::_1));
        m_WebSocket.set_fail_handler(bind(&self::OnFail, this, websocketpp::lib::placeholders::_1));
        m_WebSocket.set_message_handler(bind(&self::OnMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_WebSocket.set_close_handler(bind(&self::OnClose, this, websocketpp::lib::placeholders::_1));

        // Connection Ptr
        printf_s("%sGet connection ptr...\n", THIS_PREFIX);
        ws_lib::error_code ec; 
        m_Connection_ptr = m_WebSocket.get_connection(g_Socket_Url, ec);
        if (ec)
        {
            m_bConnecting = false;
            smutils->LogError(myself, "[FATAL ERROR]  Failed to create connection_hdl to '%s': %s", g_Socket_Url.c_str(), ec.message().c_str());
            gamehelpers->ServerCommand("exit");
            return;
        }

        // Connect
        Connect(false);
    }

    bool Connect(bool reconnect = true)
    {
    retry:
        if (m_bClosing)
            return false;

        // sleep
        threader->ThreadSleep(reconnect ? 60000u : 5000u);

        // Connection
        try
        {
            // flag
            m_bConnecting = true;
            printf_s("%sSocket connecting to \"%s\"...\n", THIS_PREFIX, g_Socket_Url.c_str());

            // go
            m_WebSocket.connect(m_Connection_ptr);

            // ASIO
            m_WebSocket.run();
        }
        catch (websocketpp::exception const &e)
        {
            smutils->LogError(myself, "Socket exception: %s", e.what());
            threader->ThreadSleep(5000u);
            goto retry;
        }

        return true;
    }

    void Disconnect()
    {
        m_bClosing = true;

        if (m_bConnected)
        {
            m_bConnected = false;
            //m_WebSocket.close(g_Connection, opcode::close, "DISCONNECT");
        }
        if (!m_WebSocket.stopped())
        {
            m_WebSocket.stop();
            smutils->LogMessage(myself, "Stopped socket.");
        }
    }

    void OnOpen(connection_hdl hdl)
    {
        m_bConnecting = false;
        m_bConnected = true;
        m_Connection_hdl = hdl;

        smutils->LogMessage(myself, "Socket conneted to \"%s\".", g_Socket_Url.c_str());

        m_WebSocket.send(m_Connection_hdl, g_Socket_Key, opcode::text);

        while (!m_bQueue.empty())
        {
            string data = m_bQueue.front();
            m_bQueue.pop();
            m_WebSocket.send(m_Connection_hdl, data, opcode::text);
        }

        g_LastSent = time(NULL);
        g_HeartBeat = timersys->CreateTimer(&g_HeartBeatTimer, 1.0f, NULL, TIMER_FLAG_REPEAT);
    }

    void OnFail(connection_hdl hdl)
    {
        connection_ptr con = m_WebSocket.get_con_from_hdl(hdl);

        m_bConnected = false;
        m_bConnecting = false;
        smutils->LogError(myself, "Failed to connect to \"%s\" :  %s", g_Socket_Url.c_str(), con->get_ec().message().c_str());

        if (!m_WebSocket.stopped())
        {
            m_WebSocket.stop();
            smutils->LogMessage(myself, "Stopped websocket on fail.");
        }

        if (g_HeartBeat != NULL)
        {
            // reset
            timersys->KillTimer(g_HeartBeat);
            g_HeartBeat = NULL;
        }

        if (!m_bConnecting && !m_bConnecting)
        {
            // reconnect
            Init();
        }
    }

    void OnClose(connection_hdl hdl)
    {
        connection_ptr con = m_WebSocket.get_con_from_hdl(hdl);

        m_bConnected = false;
        smutils->LogMessage(myself, "Server \"%s\" closed connection_hdl: %s", g_Socket_Url.c_str(), con->get_remote_close_reason().c_str());

        if (!m_WebSocket.stopped())
        {
            m_WebSocket.stop();
            smutils->LogMessage(myself, "Stopped socket on closed.");
        }

        if (g_HeartBeat != NULL)
        {
            // reset
            timersys->KillTimer(g_HeartBeat);
            g_HeartBeat = NULL;
        }

        if (!m_bConnecting && !m_bConnecting)
        {
            // reconnect
            Init();
        }
    }

    void OnMessage(connection_hdl hdl, message_ptr msg)
    {
        if (msg->get_opcode() != opcode::text)
        {
            smutils->LogMessage(myself, "Recv '%d' from \"%s\".\nJson:\n%s", msg->get_opcode(), g_Socket_Url.c_str(), msg->get_raw_payload().c_str());
            return;
        }

        PushMessage(msg->get_payload());
    }
} wsclient;

IThreadHandle *CreateThread(float heartbeat)
{
    ThreadParams params;
    params.flags = ThreadFlags::Thread_Default;
    params.prio = ThreadPriority::ThreadPrio_Low;
    g_Interval = (time_t)std::chrono::system_clock::to_time_t(std::chrono::system_clock::time_point(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::duration<float>(heartbeat))));
    return threader->MakeThread(&wsclient, &params);
}

void Shutdown()
{
    timersys->KillTimer(g_HeartBeat);
    wsclient.Shutdown();
}

bool Send(string message)
{
    return wsclient.Send(message);
}

bool WebSocketAvailable()
{
    return wsclient.Available();
}

cell_t Native_IsConnected(IPluginContext *pContext, const cell_t *params)
{
    return WebSocketAvailable();
}
