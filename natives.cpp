#include "extension.h"
#include "websocket.h"
#include "natives.h"
#include "message.h"

const sp_nativeinfo_t MessageNatives[] =
{
    {"Message.Message",         Native_Message},
    {"Message.Type.get",        Native_MsgType},
    {"Message.Send",            Native_Send},
    {"Message.RawJson",         Native_RawJson},

    {"Message.ArraySize",       Native_ArraySize},
    {"Message.ArrayIndex.get",  Native_getArrayIndex},
    {"Message.ArrayIndex.set",  Native_setArrayIndex},
    {"Message.IsChildArray",    Native_ChildArray},

    {"Message.WriteBool",       Native_WriteBool},
    {"Message.WriteShort",      Native_WriteShort},
    {"Message.WriteInt32",      Native_WriteInt32},
    {"Message.WriteInt64",      Native_WriteInt64},
    {"Message.WriteFloat",      Native_WriteFloat},
    {"Message.WriteString",     Native_WriteString},
    {"Message.WriteArrayBegin", Native_WriteArrayBegin},
    {"Message.WriteArrayEnd",   Native_WriteArrayEnd},

    {"Message.ReadBool",        Native_ReadBool},
    {"Message.ReadShort",       Native_ReadShort},
    {"Message.ReadInt32",       Native_ReadInt32},
    {"Message.ReadInt64",       Native_ReadInt64},
    {"Message.ReadFloat",       Native_ReadFloat},
    {"Message.ReadString",      Native_ReadString},
    {"Message.ReadArrayBegin",  Native_ReadArrayBegin},
    {"Message.ReadArrayNext",   Native_ReadArrayNext},
    {"Message.ReadArrayEnd",    Native_ReadArrayEnd},

    {NULL, NULL}
};


// Native calls
cell_t Native_Message(IPluginContext *pContext, const cell_t *params)
{
    if (params[1] < Message_Type::Invalid || params[1] > Message_Type::MaxMessage)
    {
        // overflow
        return pContext->ThrowNativeError("Invalid Message_Type given -> %d", params[1]);
    }

    Message_Type type = (Message_Type)params[1];

    bool success = false;
    KMessage *message = new KMessage(type, success);
    if (!success)
    {
        delete message;
        return pContext->ThrowNativeError("Message type %d is undefined.", type);
    }

    HandleError he = HandleError_None;
    Handle_t handle = handlesys->CreateHandle(g_MessageHandleType, message, pContext->GetIdentity(), myself->GetIdentity(), &he);
    if (!handle || he != HandleError_None)
    {
        return pContext->ThrowNativeError("Failed to create message handle: error #%d", he);
    }
    return handle;
}

cell_t Native_MsgType(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    return (cell_t)message->m_MsgType;
}

cell_t Native_Send(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(pContext->GetIdentity(), myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    string json = message->JsonString();

    // if not close, we can re-use
    if (params[2])
    {
        // release handle
        if ((err = handlesys->FreeHandle(hndl, &sec)) != HandleError_None)
        {
            return pContext->ThrowNativeError("Failed to close Message handle %x (error %d)", hndl, err);
        }
    }

    return Send(json);
}

cell_t Native_RawJson(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    return pContext->StringToLocalUTF8(params[2], static_cast<size_t>(params[3]), message->JsonString().c_str(), NULL);
}

cell_t Native_ArraySize(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    return message->ArraySize(key);
}

cell_t Native_getArrayIndex(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    return (cell_t)message->m_ArrayIndex;
}

cell_t Native_setArrayIndex(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    message->m_ArrayIndex = params[2];
    return true;
}

cell_t Native_ChildArray(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    return message->IsChildArray(key);
}

cell_t Native_WriteBool(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    message->WriteBool(key, (bool)params[3]);
    return true;
}

cell_t Native_WriteShort(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    message->WriteShort(key, params[3]);
    return true;
}

cell_t Native_WriteInt32(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    message->WriteInt32(key, params[3]);
    return true;
}

cell_t Native_WriteInt64(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key, *val;
    pContext->LocalToString(params[2], &key);
    pContext->LocalToString(params[3], &val);

    message->WriteInt64(key, atoll(val));
    return true;
}

cell_t Native_WriteFloat(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    message->WriteFloat(key, sp_ctof(params[3]));
    return true;
}

cell_t Native_WriteString(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key, *val;
    pContext->LocalToString(params[2], &key);
    pContext->LocalToString(params[3], &val);

    message->WriteString(key, val);
    return true;
}

cell_t Native_WriteArrayBegin(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);
    message->WriteArrayBegin(key);
    return true;
}

cell_t Native_WriteArrayEnd(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    message->WriteArrayEnd();
    return true;
}

// read
cell_t Native_ReadBool(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    return message->ReadBool(key);
}

cell_t Native_ReadShort(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    return message->ReadShort(key);
}

cell_t Native_ReadInt32(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    return message->ReadInt32(key);
}

cell_t Native_ReadInt64(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    char val[32];
    sprintf(val, "%lld", message->ReadInt64(key));

    return pContext->StringToLocal(params[3], static_cast<size_t>(params[4]), val);
}

cell_t Native_ReadFloat(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    return sp_ftoc(message->ReadFloat(key));
}

cell_t Native_ReadString(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);

    return pContext->StringToLocal(params[3], static_cast<size_t>(params[4]), message->ReadString(key).c_str());
}

cell_t Native_ReadArrayBegin(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    char *key;
    pContext->LocalToString(params[2], &key);
    
    if (!message->IsChildArray(key))
    {
        // ???
        return pContext->ThrowNativeError("Child '%s' is not an array.", key);
    }

    message->ReadArrayBegin(key);
    return true;
}

cell_t Native_ReadArrayNext(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    return message->ReadArrayNext();
}

cell_t Native_ReadArrayEnd(IPluginContext *pContext, const cell_t *params)
{
    Handle_t hndl = static_cast<Handle_t>(params[1]);
    HandleError err;
    HandleSecurity sec = HandleSecurity(NULL, myself->GetIdentity());

    KMessage *message;
    if ((err = handlesys->ReadHandle(hndl, g_MessageHandleType, &sec, (void **)&message)) != HandleError_None)
    {
        // ughahahah
        return pContext->ThrowNativeError("Invalid Message handle %x (error %d)", hndl, err);
    }

    message->ReadArrayEnd();
    return true;
}
