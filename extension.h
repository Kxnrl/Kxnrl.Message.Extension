#ifndef __KMESSAGE_EXTENSION_INCLUDE__
#define __KMESSAGE_EXTENSION_INCLUDE__

#include <string>
#include "smsdk_ext.h"

class      kMessage : 
    public SDKExtension
{
public:
    // SDKExtension
    virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
    virtual void SDK_OnUnload();
};

extern std::string g_Socket_Url;
extern bool g_bRequireRestart;
extern bool g_bSocketConnects;
extern HandleType_t g_MessageHandleType;

void PushMessage(std::string message);

#define THIS_PREFIX "[Kxnrl.Message]      "

#endif // ! __KMESSAGE_EXTENSION_INCLUDE__