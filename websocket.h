#ifndef __KMESSAGE_WEBSOCKET_INCLUDE__
#define __KMESSAGE_WEBSOCKET_INCLUDE__

#include "smsdk_ext.h"

// public
IThreadHandle *CreateThread(float interval);
void Shutdown();
bool Send(std::string message);
bool WebSocketAvailable();

// native
extern const sp_nativeinfo_t WebSocketNatives[];
cell_t Native_IsConnected(IPluginContext *pContext, const cell_t *params);

#define WEBSOCKET_SERVER_ADDRESS "ws://ws.kxnrl.com/"

#endif // ! __KMESSAGE_WEBSOCKET_INCLUDE__