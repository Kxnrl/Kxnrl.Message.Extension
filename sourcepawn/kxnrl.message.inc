#if defined __kxnrl_Message_include__
    #endinput
#endif
#define __kxnrl_Message_include__

/*
 *   Kxnrl.Message.ext
 *  Copyright 2020 Kyle
 *    2020/07/01 02:37
 */

enum Message_Type
{
    Invalid = 0,

    /* Global */

    // Connections
    PingPong            = 1,
    HeartBeat           = 2,

    // Servers
    Server_Load         = 101,
    Server_Update       = 102,  // deprecated
    Server_Start        = 103,
    Server_StartMap     = 104,
    Server_EndMap       = 105,
    Server_Query        = 106,
    Server_PushError    = 107,
    Server_PushLog      = 108,
    Server_PushGithub   = 109,
    Server_PushChatLog  = 110,
    Server_PushClients  = 111,
    Server_PushEvents   = 112,
    Server_PushMaps     = 113,
    Server_LoadMaps     = 114,
    Server_SendClient   = 115,
    Server_PushMapCD    = 116,
    Server_QueryMapCD   = 117,

    // Forums
    Forums_LoadUser     = 201,
    Forums_LoadAll      = 202,

    // Broadcast
    Broadcast_Chat      = 301,
    Broadcast_Admin     = 302,
    Broadcast_QQBot     = 303,
    Broadcast_Wedding   = 304,
    Broadcast_Other     = 305,
    Broadcast_NextMap   = 306,
    Broadcast_Horn      = 307,

    // Baning
    Ban_LoadAdmins      = 401,
    Ban_LoadAll         = 402,
    Ban_CheckUser       = 403,
    Ban_InsertIdentity  = 404,
    Ban_InsertComms     = 405,
    Ban_UnbanIdentity   = 406,
    Ban_UnbanComms      = 407,
    Ban_RefreshAdmins   = 408,
    Ban_LogAdminAction  = 409,
    Ban_LogBlocks       = 410,

    // Couples
    Couple_LoadAll      = 501,
    Couple_LoadUser     = 502,
    Couple_Update       = 503,
    Couple_Wedding      = 504,
    Couple_Divorce      = 505,
    Couple_MarriageSeek = 506,

    /* VIP/Donator */
    Vip_LoadUser        = 601,
    Vip_LoadAll         = 602,
    Vip_FromClient      = 603,
    Vip_Purchase        = 604,

    // User options
    Opts_LoadUser       = 701,
    Opts_SaveUser       = 702,

    // Authorized
    Auth_GetAuthList    = 801,
    Auth_GetUserAuth    = 802,
    Auth_GetAll         = 803,
    Auth_RequestAuth    = 804,

    /* Store */
    // Load
    Store_Load          = 900,
    Store_LoadUser      = 901,

    // Credits
    Store_EarnCredits   = 911,
    Store_CostCredits   = 912,
    Store_SetsCredits   = 913,

    // Items
    Store_PurchaseItem  = 921,
    Store_SellItem      = 922,
    Store_GiveItem      = 923,
    Store_GiftItem      = 924,
    Store_ExtendItem    = 925,
    Store_RemoveItem    = 926,
    
    // Equipments
    Store_EquipItem     = 931,
    Store_UnequipItem   = 932,

    // Extends
    Store_ExtraLog      = 941,

    /* Analytics */
    
    // Global
    Stats_LoadUser      = 1001,
    Stats_Analytics     = 1002, // deprecated
    Stats_Update        = 1003,
    Stats_DailySignIn   = 1004,

    // CSGO->MiniGames
    Ranking_MG_LoadUser = 1101,
    Ranking_MG_Update   = 1102,
    Ranking_MG_Session  = 1103,
    Ranking_MG_Trace    = 1104,
    Ranking_MG_Ranking  = 1105,
    Ranking_MG_Details  = 1106,

    // CSGO->ZombieEscape
    Ranking_ZE_LoadUser = 1111,
    Ranking_ZE_Update   = 1112,
    Ranking_ZE_Session  = 1113,
    Ranking_ZE_Ranking  = 1114,
    Ranking_ZE_Details  = 1115,

    // CSGO->TTT
    Ranking_TT_LoadUser = 1121,
    Ranking_TT_Update   = 1122,
    Ranking_TT_Session  = 1123,

    // CSGO->JB
    Ranking_JB_LoadUser = 1131,
    Ranking_JB_Update   = 1132,
    Ranking_JB_Session  = 1133,
    Ranking_JB_Ranking  = 1134,

    // End
    MaxMessage          = 2000
};

/**
 * Check service is available.
 */
native bool KxnrlMessage_IsConnected();

/**
 * Methodmap for the Kxnrl.Message extension.
 */
methodmap Message < Handle
{
    // 构造函数
    public native Message(Message_Type type);

    // 消息类型
    property Message_Type Type
    {
        // getter
        public native get();
    }

    // 发送消息 (参数为true将会自动销毁此实例, 若不销毁可以重复使用)
    public native bool Send(bool destroy = true);

    // 原始Json
    public native void RawJson(char[] buffer, int maxLen);

    // 检查数组长度
    public native int ArraySize(const char[] key);

    // 子对象是否是数组
    public native bool IsChildArray(const char[] key);

    // 设置当前数组位置
    property int ArrayIndex
    {
        public native get();
        public native set(int index);
    }

    // 写入
    public native void WriteBool(const char[] key, bool value);
    public native void WriteShort(const char[] key, int value);
    public native void WriteInt32(const char[] key, int value);
    public native void WriteInt64(const char[] key, const char[] value);
    public native void WriteFloat(const char[] key, float value);
    public native void WriteString(const char[] key, const char[] value);
    public native void WriteArrayBegin(const char[] key);
    public native void WriteArrayEnd();

    // 读取
    public native bool  ReadBool(const char[] key);
    public native int   ReadShort(const char[] key);
    public native int   ReadInt32(const char[] key);
    public native void  ReadInt64(const char[] key, char[] value, int maxLen);
    public native float ReadFloat(const char[] key);
    public native void  ReadString(const char[] key, char[] value, int maxLen);
    public native void  ReadArrayBegin(const char[] key);
    public native bool  ReadArrayNext();
    public native void  ReadArrayEnd();
}

/**
 * Called when message received.
 */
forward void OnMessageReceived(Message msg);

/**
 * Called when BlueBerry Blitz server connected.
 */
forward void OnSocketConnected();


public Extension __ext__Kxnrl_Message = 
{
    name = "Kxnrl.Message",
    file = "kxnrl.message.ext",
#if defined AUTOLOAD_EXTENSIONS
    autoload = 1,
#else
    autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};

#if !defined REQUIRE_EXTENSIONS
public void __ext_Kxnrl_Message_SetNTVOptional()
{
    MarkNativeAsOptional("KxnrlMessage_IsConnected");

    MarkNativeAsOptional("Message.Message");
    MarkNativeAsOptional("Message.Type.get");
    MarkNativeAsOptional("Message.Send");

    MarkNativeAsOptional("Message.ArraySize");
    MarkNativeAsOptional("Message.IsChildArray");
    
    MarkNativeAsOptional("Message.ArrayIndex.get");
    MarkNativeAsOptional("Message.ArrayIndex.set");

    MarkNativeAsOptional("Message.WriteBool");
    MarkNativeAsOptional("Message.WriteShort");
    MarkNativeAsOptional("Message.WriteInt32");
    MarkNativeAsOptional("Message.WriteInt64");
    MarkNativeAsOptional("Message.WriteFloat");
    MarkNativeAsOptional("Message.WriteString");
    MarkNativeAsOptional("Message.WriteArrayBegin");
    MarkNativeAsOptional("Message.WriteArrayEnd");

    MarkNativeAsOptional("Message.ReadBool");
    MarkNativeAsOptional("Message.ReadShort");
    MarkNativeAsOptional("Message.ReadInt32");
    MarkNativeAsOptional("Message.ReadInt64");
    MarkNativeAsOptional("Message.ReadFloat");
    MarkNativeAsOptional("Message.ReadString");
    MarkNativeAsOptional("Message.ReadArrayBegin");
    MarkNativeAsOptional("Message.ReadArrayNext");
    MarkNativeAsOptional("Message.ReadArrayEnd");
}
#endif