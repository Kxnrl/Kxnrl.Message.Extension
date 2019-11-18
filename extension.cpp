#include <queue>

#include "extension.h"
#include "websocket.h"
#include "message.h"
#include "natives.h"

typedef std::queue<string> tQueue;

// IThreader
IThreadHandle *websocket_thread;

// IForwardManager
IForward *g_fwdOnMessage = NULL;

// socket
string g_Socket_Url;
tQueue g_tRecvQueue;

// Ext
kMessage g_kMessage;
SMEXT_LINK(&g_kMessage);

void OnGameFrame(bool simualting);
bool g_bRequireRestart = false;

MessageTypeHandler g_MessageTypeHandler;
HandleType_t g_MessageHandleType;

bool kMessage::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
    // Forward
    printf_s("%sInit forwardsys...\n", THIS_PREFIX);
    g_fwdOnMessage = forwards->CreateForward("OnMessageReceived", ET_Ignore, 1, NULL, Param_Cell);
    if (!g_fwdOnMessage)
    {
        smutils->Format(error, maxlength, "Failed to create forward 'OnMessageReceived'.");
        return false;
    }

    // Uri
    printf_s("%sInit socket uri...\n", THIS_PREFIX);
    const char *uri = smutils->GetCoreConfigValue("WebSocket_Uri");
    if (uri == NULL)
    {
        g_Socket_Url = string(WEBSOCKET_SERVER_ADDRESS);
        smutils->LogMessage(myself, "[Connect] Key 'WebSocket_Uri' does not exists in core.cfg, connect to default server '%s'.", WEBSOCKET_SERVER_ADDRESS);
    }
    else
    {
        g_Socket_Url = uri;
        printf_s("%sSocket uri [%s]...\n", THIS_PREFIX, uri);
    }

    // Interval
    printf_s("%sInit heartbeat...\n", THIS_PREFIX);
    const char *val = smutils->GetCoreConfigValue("WebSocket_Heartbeat_Interval");
    float interval = 30.0f;
    if (val == NULL)
    {
        interval = 30.0f;
        smutils->LogMessage(myself, "[Connect] Key 'WebSocket_Heartbeat_Interval' does not exists in core.cfg, use default value '30'.");
    }
    else
    {
        interval = (float)atof(val);
        if (interval < 10.0f) interval = 10.0f;
        if (interval > 999.9f) interval = 999.9f;
        printf_s("%sSocket heartbeat interval [%.1f]...\n", THIS_PREFIX, interval);
    }

    printf_s("%sInit socket thread...\n", THIS_PREFIX);
    websocket_thread = CreateThread(interval);
    if (websocket_thread == NULL)
    {
        smutils->Format(error, maxlength, "Could not create websocket thread.");
        return false;
    }

    printf_s("%sInit handlesys...\n", THIS_PREFIX);
    g_MessageHandleType = handlesys->CreateType("Message", &g_MessageTypeHandler, 0, NULL, NULL, myself->GetIdentity(), NULL);

    sharesys->RegisterLibrary(myself, "Kxnrl.Message");

    sharesys->AddNatives(myself, WebSocketNatives);
    sharesys->AddNatives(myself, MessageNatives);

    smutils->AddGameFrameHook(&OnGameFrame);

    return true;
}

void kMessage::SDK_OnUnload()
{
    smutils->RemoveGameFrameHook(&OnGameFrame);

    if (g_fwdOnMessage)
    {
        forwards->ReleaseForward(g_fwdOnMessage);
        g_fwdOnMessage = NULL;
    }

    if (websocket_thread != NULL)
    {
        Shutdown();
        websocket_thread->WaitForThread();
        websocket_thread->DestroyThis();
    }
}

void OnGameFrame(bool simulating)
{
    if (g_bRequireRestart)
    {
        gamehelpers->ServerCommand("exit\n");
        return;
    }

begin:
    if (g_tRecvQueue.empty())
        return;

    string json = g_tRecvQueue.front();
    g_tRecvQueue.pop();

    bool success = false;
    KMessage *message = new KMessage(json, success);
    if (!success)
    {
        // loop
        delete message;
        goto begin;
    }

    Handle_t handle = handlesys->CreateHandle(g_MessageHandleType, message, NULL, myself->GetIdentity(), NULL);

    g_fwdOnMessage->PushCell(handle);
    g_fwdOnMessage->Execute();

    HandleError err;
    HandleSecurity sec;
    sec.pIdentity = myself->GetIdentity();

    if ((err = handlesys->FreeHandle(handle, &sec)) != HandleError_None)
    {
        smutils->LogError(myself, "[OnGameFrame] Failed to close Message handle %x (error %d)", handle, err);
    }
}

void PushMessage(string message)
{
    g_tRecvQueue.push(message);
}