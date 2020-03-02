#ifndef __KMESSAGE_EXTENSION_INCLUDE__
#define __KMESSAGE_EXTENSION_INCLUDE__

#include "smsdk_ext.h"

class kMessage : public SDKExtension
{
public:
    // SDKExtension
    virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
    virtual void SDK_OnUnload();
};

#endif // ! __KMESSAGE_EXTENSION_INCLUDE__