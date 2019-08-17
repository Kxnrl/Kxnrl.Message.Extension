## Kxnrl.Message.Extension
  
#### 作为游戏服务器与后台程序通信的桥梁
*** Made by Kyle "Kxnrl" Frankiss ***  
***   Copyright ©2011-2019 Kyle   ***  
   
   
### 参数
#### 以下参数需要添加到 addons/sourcemod/configs/core.cfg
* **WebSocket_Uri** - websocket后台的地址 例如:***ws://websocket.kxnrl.com***
* **WebSocket_Key** - websocket后台的密钥 验证失败将导致IP被临时封禁
* **WebSocket_Heartbeat_Interval** - websocket心跳包间隔时长, 避免因为空闲状态而掉线 ***(10.0 < value < 999.9)***
  
  
### 消息类型  
```c
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

    // Forums
    Forums_LoadUser     = 201,
    Forums_LoadAll      = 202,

    // Broadcast
    Broadcast_Chat      = 301,
    Broadcast_Admin     = 302,
    Broadcast_QQBot     = 303,
    Broadcast_Wedding   = 304,
    Broadcast_Other     = 305,

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

    /* Client */
    Client_ForwardUser  = 701,
    Client_HeartBeat    = 702,
    Client_S2S          = 703,

    /* Analytics */
    
    // Global
    Stats_LoadUser      = 1001,
    Stats_Analytics     = 1002,
    Stats_Update        = 1003,
    Stats_DailySignIn   = 1004,

    // CSGO->MiniGames
    Stats_MG_LoadUser   = 1101,
    Stats_MG_Update     = 1102,
    Stats_MG_Session    = 1103,
    Stats_MG_Trace      = 1104,
    Stats_MG_Ranking    = 1105,
    Stats_MG_Details    = 1106,

    // CSGO->ZombieEscape
    Stast_ZE_LoadUser   = 1111,
    Stast_ZE_Update     = 1112,
    Stats_ZE_Session    = 1113,
    Stats_ZE_Ranking    = 1114,
    Stats_ZE_Details    = 1115,

    // CSGO->TTT
    Stats_TT_LoadUser   = 1121,
    Stats_TT_Update     = 1122,
    Stats_TT_Session    = 1123,

    // L4D2->V
    Stats_L2_LoadUser   = 1201,
    Stats_L2_Update     = 1202,
    Stats_L2_Session    = 1203,

    // INS->PVP
    Stats_IS_LoadUser   = 1301,
    Stats_IS_Update     = 1302,
    Stats_IS_Session    = 1303,
    Stats_IS_Ranking    = 1304,
    Stats_IS_Trace      = 1305,
    Stats_IS_LoadAll    = 1306,

    // End
    MaxMessage          = 2000
};
```  
  
  
### Forward
```sourcepawn
/*
 *  收到消息时触发
 */
forward void OnMessageReceived(Message message);
```
  

### Natives
```sourcepawn
/*
 *  确认服务器是否已经建立连接
 */
native bool KxnrlMessage_IsConnected();
```
  
  
### MethodMap
```sourcepawn
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

    // 发送消息 (发送后会自动销毁handle)
    public native bool Send();

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
```


### 编译
* 需要使用Visual Studio 2017 - Windows XP (v141_xp)工具集进行编译
* 需要Boost库 >= 1.69.0 | 将文件放入boost文件夹 | 将lib放入lib文件夹
* 需要JsonCPP库 >= 1.y.z | 将文件放入json文件夹 | 将lib放入lib文件夹
* 需要WebsocketPP库 >= 0.8.1 | 将文件放入websocketpp文件夹 | 将lib放入lib文件夹
* 需要Sourcemod SDK >= 1.9 | 解压至任意文件夹 | 在系统环境变脸添加变量"SourceMod"且路径指向解压位置
* 双击/msvc15/Kxnrl.Message.sln | 按下F5编译