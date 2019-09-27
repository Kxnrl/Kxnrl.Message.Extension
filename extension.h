#ifndef __KMESSAGE_EXTENSION_INCLUDE__
#define __KMESSAGE_EXTENSION_INCLUDE__

#include <string>
#include "smsdk_ext.h"
#include "message.h"

class      kMessage : 
    public SDKExtension
{
public:
    // SDKExtension
    virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
    virtual void SDK_OnUnload();
};

class MessageTypeHandler : 
    public IHandleTypeDispatch
{
public:
    void OnHandleDestroy(HandleType_t type, void *object)
    {
        delete (KMessage *)object;
    }
};

extern string g_Socket_Url;
extern HandleType_t g_MessageHandleType;

void PushMessage(string message);

#define THIS_PREFIX "[Kxnrl.Message]      "

#endif // ! __KMESSAGE_EXTENSION_INCLUDE__