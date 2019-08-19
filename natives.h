#ifndef __KMESSAGE_NATIVES_INCLUDE__
#define __KMESSAGE_NATIVES_INCLUDE__

#include "smsdk_ext.h"

// Native calls
cell_t Native_Message(IPluginContext *pContext, const cell_t *params);
cell_t Native_MsgType(IPluginContext *pContext, const cell_t *params);
cell_t Native_Send(IPluginContext *pContext, const cell_t *params);
cell_t Native_RawJson(IPluginContext *pContext, const cell_t *params);

cell_t Native_ArraySize(IPluginContext *pContext, const cell_t *params);
cell_t Native_getArrayIndex(IPluginContext *pContext, const cell_t *params);
cell_t Native_setArrayIndex(IPluginContext *pContext, const cell_t *params);
cell_t Native_ChildArray(IPluginContext *pContext, const cell_t *params);

// write
cell_t Native_WriteBool      (IPluginContext *pContext, const cell_t *params);
cell_t Native_WriteShort     (IPluginContext *pContext, const cell_t *params);
cell_t Native_WriteInt32     (IPluginContext *pContext, const cell_t *params);
cell_t Native_WriteInt64     (IPluginContext *pContext, const cell_t *params);
cell_t Native_WriteFloat     (IPluginContext *pContext, const cell_t *params);
cell_t Native_WriteString    (IPluginContext *pContext, const cell_t *params);
cell_t Native_WriteArrayBegin(IPluginContext *pContext, const cell_t *params);
cell_t Native_WriteArrayEnd  (IPluginContext *pContext, const cell_t *params);

// read
cell_t Native_ReadBool      (IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadShort     (IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadInt32     (IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadInt64     (IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadFloat     (IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadString    (IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadArrayBegin(IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadArrayNext (IPluginContext *pContext, const cell_t *params);
cell_t Native_ReadArrayEnd  (IPluginContext *pContext, const cell_t *params);

extern const sp_nativeinfo_t MessageNatives[];

#endif // ! __KMESSAGE_NATIVES_INCLUDE__