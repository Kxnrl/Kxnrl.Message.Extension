#ifndef __KMESSAGE_MESSAGE_INCLUDE__
#define __KMESSAGE_MESSAGE_INCLUDE__

#include <string>
#include "json/json.h"
#include "smsdk_ext.h"

typedef std::string string;

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

    // CSGO->FFA
    Ranking_FA_LoadUser = 1141,
    Ranking_FA_Update   = 1142,
    Ranking_FA_Session  = 1143,
    Ranking_FA_Ranking  = 1144,

    // CSGO->Casual
    Ranking_CS_LoadUser = 1151,
    Ranking_CS_Update   = 1152,
    Ranking_CS_Session  = 1153,
    Ranking_CS_Ranking  = 1154,

    // Acts
    Acts_LoadUser       = 1990,
    Acts_SyncUser       = 1991,
    Acts_SignUser       = 1992,
    Acts_Update         = 1993,
    Acts_Exchange       = 1994,
    Acts_Accomplished   = 1995,
    Acts_LoadTask       = 1996,
    Acts_SyncTask       = 1997,

    // End
    MaxMessage          = 2000
};


class KMessage
{

public:
    // Type
    Message_Type m_MsgType;

private:
    // Raw
    Json::Value m_RawJson;

    // Array
    bool m_ArrayMode;
    char m_ArrayKey[32];

public:
    Json::Value::ArrayIndex m_ArrayIndex;

public:

    string JsonString(bool pretty = false)
    {
        Json::StreamWriterBuilder m_Writer;
        m_Writer["indentation"] = pretty ? "    " : "";
        return Json::writeString(m_Writer, m_RawJson);
    };

    // create from self
    KMessage(Message_Type type)
    {
        m_ArrayMode = false;
        m_ArrayIndex = 0;
        m_MsgType = type;
        m_RawJson["Message_Type"] = (int16_t)m_MsgType;
    }

    // create from plugin
    KMessage(Message_Type type, bool &success)
    {
        if (!IsValidType(type))
        {
            success = false;
            return;
        }

        m_ArrayMode = false;
        m_ArrayIndex = 0;
        m_MsgType = type;
        m_RawJson["Message_Type"] = (int16_t)m_MsgType;
        success = true;
    }

    // recv from server
    KMessage(string json, bool &success)
    {
        Json::CharReaderBuilder m_Builder;
        Json::CharReader *m_Reader = m_Builder.newCharReader();

        string errors;
        success = m_Reader->parse(json.c_str(), json.c_str() + json.size(), &m_RawJson, &errors);
        delete m_Reader;

        if (!success)
        {
            smutils->LogError(myself, "Failed to render json from string: %s\nJson String:\n%s", errors.c_str(), json.c_str());
            return;
        }

        m_MsgType = (Message_Type)m_RawJson["Message_Type"].asInt();

        if (!IsValidType(m_MsgType))
        {
            success = false;
            smutils->LogError(myself, "Failed to crate message: type %d is undefined.\nJson String:\n%s", m_MsgType, json.c_str());
            return;
        }

        m_ArrayMode = false;
        m_ArrayIndex = 0;
    }

    ~KMessage()
    {
        // ...
    }

    int32_t ArraySize(const char *key)
    {
        return m_RawJson["Message_Data"][key].size();
    }

    void JumpArray(int32_t index)
    {
        m_ArrayIndex = index;
    }

    bool IsChildArray(const char *key)
    {
        return m_RawJson["Message_Data"][key].isArray();
    }

    /*
     *   Write Function
     */
    void WriteBool(const char *key, bool value)
    {
        if (m_ArrayMode)
        {
            if (strlen(key) > 0)
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
            }
            else
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex] = value;
            }
        }
        else
        {
            m_RawJson["Message_Data"][key] = value;
        }
    }

    void WriteShort(const char *key, int16_t value)
    {
        if (m_ArrayMode)
        {
            if (strlen(key) > 0)
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
            }
            else
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex] = value;
            }
        }
        else
        {
            m_RawJson["Message_Data"][key] = value;
        }
    }

    void WriteInt32(const char *key, int32_t value)
    {
        if (m_ArrayMode)
        {
            if (strlen(key) > 0)
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
            }
            else
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex] = value;
            }
        }
        else
        {
            m_RawJson["Message_Data"][key] = value;
        }
    }

    void WriteInt64(const char *key, int64_t value)
    {
        if (m_ArrayMode)
        {
            if (strlen(key) > 0)
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
            }
            else
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex] = value;
            }
        }
        else
        {
            m_RawJson["Message_Data"][key] = value;
        }
    }

    void WriteFloat(const char *key, float value)
    {
        if (m_ArrayMode)
        {
            if (strlen(key) > 0)
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
            }
            else
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex] = value;
            }
        }
        else
        {
            m_RawJson["Message_Data"][key] = value;
        }
    }

    void WriteString(const char *key, const char *value)
    {
        if (m_ArrayMode)
        {
            if (strlen(key) > 0)
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
            }
            else
            {
                m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex] = value;
            }
        }
        else
        {
            m_RawJson["Message_Data"][key] = value;
        }
    }

    void WriteArrayBegin(const char *key)
    {
        m_ArrayMode = true;
        m_ArrayIndex = 0;

        strncpy(m_ArrayKey, key, sizeof(m_ArrayKey));
        m_ArrayKey[sizeof(m_ArrayKey)-1] = '\0';
    }

    void WriteArrayEnd()
    {
        m_ArrayMode = false;
    }

    /*
     *   Read Function
     */
    bool ReadBool(const char *key)
    {
        Json::Value json = m_ArrayMode ? (strlen(key) > 0 ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex]) : m_RawJson["Message_Data"][key];

        if (json.isConvertibleTo(Json::ValueType::booleanValue))
        {
            return json.asBool();
        }
        else if (json.isString())
        {
            // STRING VALUE
            return (stricmp("true", json.asCString()) == 0);
        }

        return false;
    }

    int16_t ReadShort(const char *key)
    {
        Json::Value json = m_ArrayMode ? (strlen(key) > 0 ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex]) : m_RawJson["Message_Data"][key];

        if (!json.isNumeric())
        {
            if (json.isConvertibleTo(Json::ValueType::intValue))
            {
                return (int16_t)json.asInt();
            }
            else if (json.isConvertibleTo(Json::ValueType::stringValue))
            {
                return (int16_t)atoi(json.asCString());
            }
        }

        return (int16_t)json.asInt();
    }

    int32_t ReadInt32(const char *key)
    {
        Json::Value json = m_ArrayMode ? (strlen(key) > 0 ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex]) : m_RawJson["Message_Data"][key];

        if (!json.isNumeric())
        {
            if (json.isConvertibleTo(Json::ValueType::intValue))
            {
                return (int32_t)json.asInt();
            }
            else if (json.isConvertibleTo(Json::ValueType::stringValue))
            {
                return (int32_t)atoi(json.asCString());
            }
        }

        return (int32_t)json.asInt();
    }

    int64_t ReadInt64(const char *key)
    {
        Json::Value json = m_ArrayMode ? (strlen(key) > 0 ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex]) : m_RawJson["Message_Data"][key];
        return json.isInt64() ? json.asInt64() : atoll(json.asCString());
    }

    float ReadFloat(const char *key)
    {
        Json::Value json = m_ArrayMode ? (strlen(key) > 0 ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex]) : m_RawJson["Message_Data"][key];
        return json.asFloat();
    }

    string ReadString(const char *key)
    {
        Json::Value json = m_ArrayMode ? (strlen(key) > 0 ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex]) : m_RawJson["Message_Data"][key];
        return json.asString();
    }

    void ReadArrayBegin(const char *key)
    {
        m_ArrayMode = true;
        m_ArrayIndex = 0;

        strncpy(m_ArrayKey, key, sizeof(m_ArrayKey));
        m_ArrayKey[sizeof(m_ArrayKey)-1] = '\0';
    }

    bool ReadArrayNext()
    {
        return m_RawJson["Message_Data"][m_ArrayKey].isValidIndex(++m_ArrayIndex);
    }

    void ReadArrayEnd()
    {
        m_ArrayMode = false;
    }

private:

    bool IsValidType(Message_Type type)
    {
        switch (type)
        {
        case Message_Type::PingPong:
        case Message_Type::HeartBeat:
        case Message_Type::Server_Load:
        case Message_Type::Server_Update:
        case Message_Type::Server_Start:
        case Message_Type::Server_StartMap:
        case Message_Type::Server_EndMap:
        case Message_Type::Server_Query:
        case Message_Type::Server_PushError:
        case Message_Type::Server_PushLog:
        case Message_Type::Server_PushGithub:
        case Message_Type::Server_PushChatLog:
        case Message_Type::Server_PushClients:
        case Message_Type::Server_PushEvents:
        case Message_Type::Server_PushMaps:
        case Message_Type::Server_LoadMaps:
        case Message_Type::Server_SendClient:
        case Message_Type::Forums_LoadUser:
        case Message_Type::Forums_LoadAll:
        case Message_Type::Broadcast_Chat:
        case Message_Type::Broadcast_Admin:
        case Message_Type::Broadcast_QQBot:
        case Message_Type::Broadcast_Wedding:
        case Message_Type::Broadcast_Other:
        case Message_Type::Broadcast_NextMap:
        case Message_Type::Broadcast_Horn:
        case Message_Type::Ban_LoadAdmins:
        case Message_Type::Ban_LoadAll:
        case Message_Type::Ban_CheckUser:
        case Message_Type::Ban_InsertIdentity:
        case Message_Type::Ban_InsertComms:
        case Message_Type::Ban_UnbanIdentity:
        case Message_Type::Ban_UnbanComms:
        case Message_Type::Ban_RefreshAdmins:
        case Message_Type::Ban_LogAdminAction:
        case Message_Type::Ban_LogBlocks:
        case Message_Type::Couple_LoadAll:
        case Message_Type::Couple_LoadUser:
        case Message_Type::Couple_Update:
        case Message_Type::Couple_Wedding:
        case Message_Type::Couple_Divorce:
        case Message_Type::Couple_MarriageSeek:
        case Message_Type::Vip_LoadUser:
        case Message_Type::Vip_LoadAll:
        case Message_Type::Vip_FromClient:
        case Message_Type::Vip_Purchase:
        case Message_Type::Opts_LoadUser:
        case Message_Type::Opts_SaveUser:
        case Message_Type::Auth_GetAuthList:
        case Message_Type::Auth_GetUserAuth:
        case Message_Type::Auth_GetAll:
        case Message_Type::Auth_RequestAuth:
        case Message_Type::Store_Load:
        case Message_Type::Store_LoadUser:
        case Message_Type::Store_EarnCredits:
        case Message_Type::Store_CostCredits:
        case Message_Type::Store_SetsCredits:
        case Message_Type::Store_PurchaseItem:
        case Message_Type::Store_SellItem:
        case Message_Type::Store_GiveItem:
        case Message_Type::Store_GiftItem:
        case Message_Type::Store_ExtendItem:
        case Message_Type::Store_RemoveItem:
        case Message_Type::Store_EquipItem:
        case Message_Type::Store_UnequipItem:
        case Message_Type::Store_ExtraLog:
        case Message_Type::Stats_LoadUser:
        case Message_Type::Stats_Analytics:
        case Message_Type::Stats_Update:
        case Message_Type::Stats_DailySignIn:
        case Message_Type::Ranking_MG_LoadUser:
        case Message_Type::Ranking_MG_Update:
        case Message_Type::Ranking_MG_Session:
        case Message_Type::Ranking_MG_Trace:
        case Message_Type::Ranking_MG_Ranking:
        case Message_Type::Ranking_MG_Details:
        case Message_Type::Ranking_ZE_LoadUser:
        case Message_Type::Ranking_ZE_Update:
        case Message_Type::Ranking_ZE_Session:
        case Message_Type::Ranking_ZE_Ranking:
        case Message_Type::Ranking_ZE_Details:
        case Message_Type::Ranking_TT_LoadUser:
        case Message_Type::Ranking_TT_Update:
        case Message_Type::Ranking_TT_Session:
        case Message_Type::Ranking_JB_LoadUser:
        case Message_Type::Ranking_JB_Update:
        case Message_Type::Ranking_JB_Session:
        case Message_Type::Ranking_JB_Ranking:
        case Message_Type::Ranking_FA_LoadUser:
        case Message_Type::Ranking_FA_Update:
        case Message_Type::Ranking_FA_Session:
        case Message_Type::Ranking_FA_Ranking:
        case Message_Type::Ranking_CS_LoadUser:
        case Message_Type::Ranking_CS_Update:
        case Message_Type::Ranking_CS_Session:
        case Message_Type::Ranking_CS_Ranking:
        case Message_Type::Acts_LoadUser:
        case Message_Type::Acts_SyncUser:
        case Message_Type::Acts_SignUser:
        case Message_Type::Acts_Update:
        case Message_Type::Acts_Exchange:
        case Message_Type::Acts_Accomplished:
        case Message_Type::Acts_LoadTask:
        case Message_Type::Acts_SyncTask:
            return true;
        }

        return false;
    }
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

#endif // ! __KMESSAGE_MESSAGE_INCLUDE__