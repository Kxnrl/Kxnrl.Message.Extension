#include <string>
#include <memory>
#include <vector>

#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "extension.h"
#include "websocket.h"
#include "message.h"
#include "natives.h"


kMessage g_kMessage;
SMEXT_LINK(&g_kMessage);

namespace beast = boost::beast;

std::unique_ptr<boost::asio::io_context> g_IoContext;
std::unique_ptr<boost::asio::io_context> g_GameContext;

std::unique_ptr<boost::thread> g_pIoThread = nullptr;
std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> g_pIoThreadWork = nullptr;

std::unique_ptr<WSClient<beast::ssl_stream<beast::tcp_stream>>> g_pTlsClient = nullptr;
std::unique_ptr<WSClient<beast::tcp_stream>> g_pClient = nullptr;

std::string g_Socket_Url;
IForward *g_fwdOnMessage = nullptr;

MessageTypeHandler g_MessageTypeHandler;
HandleType_t g_MessageHandleType;

boost::atomic_float_t g_fInterval = 30.0;

void OnGameFrame(bool simualting);

void SetClientWithUri(std::string uri)
{
    bool ssl = false;
    std::string port("80");
    std::string path("/");
    if (boost::algorithm::starts_with(uri, "wss://")) {
        ssl = true;

        // sizeof("wss://") == 6
        uri = uri.substr(6);
        port = "443";
    }
    else if (boost::algorithm::starts_with(uri, "ws://")) {
        // sizeof("ws://") == 5
        uri = uri.substr(5);
    }

    std::string hostport = uri;

    size_t pos = uri.find('/');
    if (pos != std::string::npos) {
        hostport = uri.substr(0, pos);
        path = uri.substr(pos);
    }

    pos = hostport.find(':');
    std::string host = uri.substr(0, pos);
    if (pos != std::string::npos)
        port = uri.substr(pos+1);

    smutils->LogMessage(myself, "Connect to %s:%s%s", host.c_str(), port.c_str(), path.c_str());

    if (ssl) {
        g_pTlsClient = std::make_unique<WSClient<beast::ssl_stream<beast::tcp_stream>>>(host, port, path, *g_IoContext, *g_GameContext, g_fInterval.load());
        g_pTlsClient->Start();
    }
    else {
        g_pClient = std::make_unique<WSClient<beast::tcp_stream>>(host, port, path, *g_IoContext, *g_GameContext, g_fInterval.load());
        g_pClient->Start();
    }
}

bool kMessage::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
    g_fwdOnMessage = forwards->CreateForward("OnMessageReceived", ET_Ignore, 1, NULL, Param_Cell);
    if (g_fwdOnMessage == nullptr)
    {
        smutils->Format(error, maxlength, "Failed to create forward \"OnMessageReceived\"");
        return false;
    }

    // Uri
    const char *uri = smutils->GetCoreConfigValue("WebSocket_Uri");
    if (uri == nullptr)
    {
        g_Socket_Url = std::move(std::string(WEBSOCKET_SERVER_ADDRESS));
        smutils->LogMessage(myself, "Key \"WebSocket_Uri\" does not exist in core.cfg, connect to default server \"%s\" instead.", WEBSOCKET_SERVER_ADDRESS);
    }
    else
    {
        g_Socket_Url = uri;
        smutils->LogMessage(myself, "Socket URI: %s", uri);
    }

    const char *val = smutils->GetCoreConfigValue("WebSocket_Heartbeat_Interval");
    if (val == nullptr)
    {
        smutils->LogMessage(myself, "Key \"WebSocket_Heartbeat_Interval\" does not exist in core.cfg, use default value \"%.1f\".", g_fInterval.load());
    }
    else
    {
        g_fInterval = boost::lexical_cast<float>(val);
        if (g_fInterval < 10.0f)
            g_fInterval = 10.0f;

        smutils->LogMessage(myself, "Socket heartbeat initialized with interval %.1f.", g_fInterval.load());
    }

    g_MessageHandleType = handlesys->CreateType("Message", &g_MessageTypeHandler, 0, NULL, NULL, myself->GetIdentity(), NULL);

    // Initialize IO here.
    g_IoContext = std::make_unique<boost::asio::io_context>();
    g_GameContext = std::make_unique<boost::asio::io_context>();
    SetClientWithUri(g_Socket_Url);

    sharesys->RegisterLibrary(myself, "Kxnrl.Message");

    sharesys->AddNatives(myself, WebSocketNatives);
    sharesys->AddNatives(myself, MessageNatives);

    smutils->AddGameFrameHook(&OnGameFrame);

    g_pIoThread = std::make_unique<boost::thread>([] {
        g_pIoThreadWork = std::make_unique<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>>(boost::asio::make_work_guard(*g_IoContext));
        g_IoContext->run();
        g_pIoThreadWork = nullptr;
    });

    return true;
}

void CleanupClient()
{
    if (g_pTlsClient) {
        g_pTlsClient->Stop();
        g_pTlsClient = nullptr;
    }

    if (g_pClient) {
        g_pClient->Stop();
        g_pClient = nullptr;
    }
}

void kMessage::SDK_OnUnload()
{
    smutils->RemoveGameFrameHook(&OnGameFrame);

    CleanupClient();

    if (g_pIoThread)
    {
        if (g_pIoThreadWork) {
            g_pIoThreadWork->reset();
        }
        
        g_IoContext->stop();

        g_pIoThread->join();
        g_pIoThread = nullptr;

        g_IoContext = nullptr;
    }

    g_GameContext->stop();
    g_GameContext = nullptr;

    handlesys->RemoveType(g_MessageHandleType, myself->GetIdentity());

    if (g_fwdOnMessage)
    {
        forwards->ReleaseForward(g_fwdOnMessage);
        g_fwdOnMessage = nullptr;
    }
}

void OnGameFrame(bool simulating)
{
    g_GameContext->run();
}

void pushBuffer(beast::flat_buffer buffer)
{
    bool success = false;
    auto message = new KMessage(std::string((const char*)buffer.data().data(), buffer.data().size()), success);
    if (!success)
    {
        delete message;
    }

    Handle_t handle = handlesys->CreateHandle(g_MessageHandleType, message, NULL, myself->GetIdentity(), NULL);

    g_fwdOnMessage->PushCell(handle);
    g_fwdOnMessage->Execute();

    HandleError err;
    HandleSecurity sec;
    sec.pIdentity = myself->GetIdentity();

    if ((err = handlesys->FreeHandle(handle, &sec)) != HandleError_None)
    {
        smutils->LogError(myself, "Failed to close Message handle %x (error %d)", handle, err);
    }
}

void reportError(boost::system::system_error err)
{
    smutils->LogMessage(myself, "Error: %s", err.code().message().c_str());
    smutils->LogMessage(myself, "Restarting the session");

    CleanupClient();
    SetClientWithUri(g_Socket_Url);
}

void Send(std::string &json)
{
    std::vector<uint8_t> buf(json.data(), json.data() + json.size());
    if (g_pTlsClient) {
        g_pTlsClient->Send(buf);
    }
    else if (g_pClient) {
        g_pClient->Send(buf);
    }
}

cell_t Native_IsConnected(IPluginContext *pContext, const cell_t *params)
{
    if (g_pTlsClient) {
        return g_pTlsClient->IsOpen() ? 1 : 0;
    }
    else if (g_pClient) {
        return g_pClient->IsOpen() ? 1 : 0;
    }
    return 0;
}

sp_nativeinfo_t WebSocketNatives[] =
{
    {"KxnrlMessage_IsConnected", Native_IsConnected},
    {NULL, NULL},
};
