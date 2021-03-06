#include "extension.h"
#include "websocket.h"
#include "message.h"
#include <chrono>
#include <unordered_map>
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
typedef ws_lib::error_code errcode;
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
        printf("%s", buffer);

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

    bool Send(string message, bool isResend = false)
    {
        if (!Available())
        {
            if (!isResend)
            {
                SaveQueue(message);
            }
            else
            {
                smutils->LogError(myself, "Resend message still falling because ready state error.");
            }
            return false;
        }

        g_LastSent = time(NULL);
        errcode ec;
        m_WebSocket.send(m_Connection_hdl, message, opcode::text, ec);
        if (ec)
        {
            if (!isResend)
            {
                SaveQueue(message);
            }  
            smutils->LogError(myself, "Failed to send message to server: %s -> [%s]", ec.message().c_str(), message.c_str());
            return false;
        }
        return true;
    }

    void Shutdown()
    {
        m_bClosing = true;
        printf("%s Shutdown websocket client.\n", THIS_PREFIX);

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
    void SaveQueue(string message)
    {
        if (m_bQueue.find(message) == m_bQueue.end())
        {
            m_bQueue[message] = true;
            smutils->LogMessage(myself, "Socket is unavailable now. Push data to queue. -> %s", message.c_str());
        }
        else
        {
            smutils->LogMessage(myself, "Socket is unavailable now. but data is already in queue. -> %s", message.c_str());
        }
    }

    void PushQueue()
    {
        // push all local storage
        if (m_bQueue.empty())
            return;

        smutils->LogMessage(myself, "Begin push queue -> %d messages in queue.", m_bQueue.size());

        auto iter = m_bQueue.begin();
        while (iter != m_bQueue.end())
        {
            if (!Send(iter->first, true))
            {
                smutils->LogError(myself, "Failed to resend -> %s", iter->first.c_str());
                break;
            }
            smutils->LogMessage(myself, "Resend -> %s", iter->first.c_str());
            iter = m_bQueue.erase(iter);
        }

        smutils->LogMessage(myself, "Handled all messages in queue");
    }

private:
    client m_WebSocket;
    connection_hdl m_Connection_hdl;
    connection_ptr m_Connection_ptr;
    bool m_bConnected = false;
    bool m_bConnecting = false;
    bool m_bClosing = false;
    std::unordered_map<string, bool> m_bQueue;
    typedef WebSocketClient self;
    uint16_t m_Retries = 0;

    /* Init WebSocket */
    void Init(bool init = false)
    {
        if (init)
        {
            // ASIO
            printf("%sInit socket asio service...\n", THIS_PREFIX);
            m_WebSocket.init_asio();
        }
        else
        {
            // Reset
            printf("%sReset socket asio service...\n", THIS_PREFIX);
            m_WebSocket.reset();
        }

        // Log
        printf("%sSet socket log level...\n", THIS_PREFIX);
        m_WebSocket.set_access_channels(ws_log::alevel::none);
        m_WebSocket.set_error_channels(ws_log::elevel::fatal);
        m_WebSocket.clear_access_channels(ws_log::alevel::frame_header);
        m_WebSocket.clear_access_channels(ws_log::alevel::frame_payload);

        // User-Agent
        printf("%sSet socket user-agent...\n", THIS_PREFIX);
        m_WebSocket.set_user_agent("Kxnrl.Message Extension");

        // Params
        m_WebSocket.set_close_handshake_timeout(10000);
        m_WebSocket.set_open_handshake_timeout(10000);

        // Event Handlers
        printf("%sSet event handler...\n", THIS_PREFIX);
        m_WebSocket.set_open_handler(bind(&self::OnOpen, this, websocketpp::lib::placeholders::_1));
        m_WebSocket.set_fail_handler(bind(&self::OnFail, this, websocketpp::lib::placeholders::_1));
        m_WebSocket.set_message_handler(bind(&self::OnMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
        m_WebSocket.set_close_handler(bind(&self::OnClose, this, websocketpp::lib::placeholders::_1));

        // Connection Ptr
        printf("%sGet connection ptr...\n", THIS_PREFIX);
        ws_lib::error_code ec; 
        m_Connection_ptr = m_WebSocket.get_connection(g_Socket_Url, ec);
        if (ec)
        {
            m_bConnecting = false;
            smutils->LogError(myself, "[FATAL ERROR]  Failed to create connection_hdl to '%s': %s", g_Socket_Url.c_str(), ec.message().c_str());
            return;
        }

        if (++m_Retries >= 50)
        {
            // restart server.
            g_bRequireRestart = true;
            return;
        }

        // Connect
        Connect(!init);
    }

    bool Connect(bool reconnect = true)
    {
    retry:
        if (m_bClosing)
        {
            printf("%s OnClosing, DO NOT make connection.\n", THIS_PREFIX);
            return false;
        }

        // sleep
        threader->ThreadSleep(reconnect ? 8000u : 10u);

        // Connection
        try
        {
            // flag
            m_bConnecting = true;
            printf("%s #%d Socket connecting to \"%s\"...\n", THIS_PREFIX, m_Retries, g_Socket_Url.c_str());

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
        printf("%s Disconnecting...\n", THIS_PREFIX);

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
        m_Retries = 0;
        m_bConnecting = false;
        m_bConnected = true;
        m_Connection_hdl = hdl;
        g_bSocketConnects = true;

        smutils->LogMessage(myself, "Socket conneted to \"%s\".", g_Socket_Url.c_str());

        // push all local storage
        PushQueue();

        g_LastSent = time(NULL);
        g_HeartBeat = timersys->CreateTimer(&g_HeartBeatTimer, 1.0f, NULL, TIMER_FLAG_REPEAT);
        smutils->LogMessage(myself, "Starting heartbeat timer.");
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
        smutils->LogError(myself, "Server \"%s\" closed connection_hdl: %s", g_Socket_Url.c_str(), con->get_remote_close_reason().c_str());

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
            smutils->LogError(myself, "Recv '%d' from \"%s\".\nJson:\n%s", msg->get_opcode(), g_Socket_Url.c_str(), msg->get_raw_payload().c_str());
            return;
        }

        PushMessage(msg->get_payload());

        PushQueue();
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
