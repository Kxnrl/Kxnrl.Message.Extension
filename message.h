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

    string JsonString() const
    {
        Json::StreamWriterBuilder m_Writer;
        m_Writer["indentation"] = "";
        return Json::writeString(m_Writer, m_RawJson);
    };

    // create from plugin
    KMessage(Message_Type type)
    {
        m_MsgType = type;
        m_RawJson["Message_Type"] = Message_Type::Invalid;
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
    }

    ~KMessage()
    {
        // ...
    }

    int32_t ArraySize(const char *key)
    {
        if (!m_ArrayMode || !m_ArrayKey[0])
        {
            return 0;
        }

        return m_RawJson["Message_Data"][m_ArrayKey].size();
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
            m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
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
            m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
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
            m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
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
            m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
        }
        else
        {
            m_RawJson["Message_Data"][key] = value;
        }
    }

    void WriteFloat(const char *key, float_t value)
    {
        if (m_ArrayMode)
        {
            m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
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
            m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] = value;
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

        strcpy_s(m_ArrayKey, sizeof(m_ArrayKey), key);
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
        Json::Value json = m_ArrayMode ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][key];

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
        Json::Value json = m_ArrayMode ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][key];

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

        return 0;
    }

    int32_t ReadInt32(const char *key)
    {
        Json::Value json = m_ArrayMode ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][key];

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

        return 0;
    }

    int64_t ReadInt64(const char *key)
    {
        Json::Value json = m_ArrayMode ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][key];
        return json.isInt64() ? json.asInt64() : atoll(json.asCString());
    }

    float ReadFloat(const char *key)
    {
        Json::Value json = m_ArrayMode ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][key];
        return json.asFloat();
    }

    string ReadString(const char *key)
    {
        Json::Value json = m_ArrayMode ? m_RawJson["Message_Data"][m_ArrayKey][m_ArrayIndex][key] : m_RawJson["Message_Data"][key];
        return json.asString();
    }

    void ReadArrayBegin(const char *key)
    {
        m_ArrayMode = true;
        m_ArrayIndex = 0;

        strcpy_s(m_ArrayKey, sizeof(m_ArrayKey), key);
    }

    bool ReadArrayNext()
    {
        return m_RawJson["Message_Data"].isValidIndex(++m_ArrayIndex);
    }

    void ReadArrayEnd()
    {
        m_ArrayMode = true;
    }
};

#endif // ! __KMESSAGE_MESSAGE_INCLUDE__