#include "Processor.h"
#include "globe.h"
#include "LogComm.h"
#include "DataProxyProto.h"
#include "ServiceDefine.h"
#include "util/tc_hash_fun.h"
#include "uuid.h"
#include "SocialServer.h"
#include "util/tc_base64.h"

//
using namespace std;
using namespace dataproxy;
using namespace dbagent;

//过期时间
#define GIFT_EXPIRED_TIME (24*60*60)

/**
 *
*/
Processor::Processor()
{
}

/**
 *
*/
Processor::~Processor()
{

}

// CHAT_EXT_INFO  = 49,    //扩展的聊天数据
//查询
int Processor::selectChatExtInfo(const string &number, TicketInfo &info)
{
    TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_STRING) + ":" + I2S(CHAT_EXT_INFO) + ":" + number;
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(number);

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "content";
    fields.push_back(tfield);
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(number)->redisRead(dataReq, dataRsp);
    if (iRet != 0)
    {
        return -1;
    }

    ROLLLOG_DEBUG << "read cache, req: " << printTars(dataReq) << ", rsp: " << printTars(dataRsp) << endl;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfield = it->begin(); itfield != it->end(); ++itfield)
        {
            //门票名称
            if (itfield->colName == "content")
            {
                __TRY__
                toObj(TC_Base64::decode(itfield->colValue), info);
                __CATCH__
            }
        }
    }

    return 0;
}

//添加
int Processor::addChatExtInfo(const string &number, const TicketInfo &info)
{
    dataproxy::TWriteDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_STRING) + ":" + I2S(CHAT_EXT_INFO) + ":" + number;
    dataReq.operateType = E_REDIS_INSERT;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(number);
    // dataReq.paraExt.resetDefautlt();
    // dataReq.paraExt.queryType = E_REPLACE;

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "transaction_id";
    tfield.colType = dbagent::STRING;
    tfield.colValue = number;
    fields.push_back(tfield);
    tfield.colName = "content";
    tfield.colType = dbagent::STRING;
    tfield.colValue = TC_Base64::encode(tostring(info));
    fields.push_back(tfield);
    dataReq.fields = fields;

    dataproxy::TWriteDataRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(number)->redisWrite(dataReq, dataRsp);
    ROLLLOG_DEBUG << "addChatExtInfo, iRet: " << iRet << ", dataRsp: " << printTars(dataRsp) << endl;
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "addChatExtInfo err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    return 0;
}

//更新
int Processor::updateChatExtInfo(const string &number, const TicketInfo &info)
{
    dataproxy::TWriteDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_STRING) + ":" + I2S(CHAT_EXT_INFO) + ":" + number;
    dataReq.operateType = E_REDIS_WRITE;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(number);

    vector<dbagent::TField> fields;
    dbagent::TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "transaction_id";
    tfield.colType = dbagent::STRING;
    tfield.colValue = number;
    fields.push_back(tfield);
    tfield.colName = "content";
    tfield.colType = dbagent::STRING;
    tfield.colValue = TC_Base64::encode(tostring(info));
    fields.push_back(tfield);
    dataReq.fields = fields;

    dataproxy::TWriteDataRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(number)->redisWrite(dataReq, dataRsp);
    ROLLLOG_DEBUG << "updateChatExtInfo, iRet: " << iRet << ", dataRsp: " << printTars(dataRsp) << endl;
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "updateChatExtInfo err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -2;
    }

    return 0;
}

int Processor::getGiveChipsTime(tars::Int64 uid, tars::Int64 friend_uid, int &give_time)
{
    FUNC_ENTRY("");
    int iRet = 0;
    __TRY__

    if ((uid <= 0) || (friend_uid <= 0))
    {
        ROLLLOG_ERROR << "uid: " << uid << " or friend_uid: " << friend_uid << " error!" << endl;
        return -1;
    }

    dataproxy::TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_LIST) + ":" + I2S(FRIEND_INFO) + ":" + L2S(uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_USER_ID;
    dataReq.clusterInfo.frageFactor = uid;
    dataReq.paraExt.resetDefautlt();
    dataReq.paraExt.subOperateType = E_REDIS_LIST_RANGE;
    dataReq.paraExt.start = 0;//起始下标从0开始
    dataReq.paraExt.end = -1;//终止最大结束下标为-1

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "friend_uid";
    fields.push_back(tfield);
    tfield.colName = "give_time";
    fields.push_back(tfield);
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(uid)->redisRead(dataReq, dataRsp);
    if (iRet != 0 || dataRsp.iResult != 0)
    {
        ROLLLOG_ERROR << "get friends give info err, iRet: " << iRet << ", iResult: " << dataRsp.iResult << endl;
        return -1;
    }

    give_time = 0;
    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        long lFriendUid = 0;
        int iGiveTime = 0;
        for (auto itTField = it->begin(); itTField != it->end(); ++itTField)
        {
            if (itTField->colName == "friend_uid")
            {
                lFriendUid = S2L(itTField->colValue);
            }
            else if (itTField->colName == "give_time")
            {
                iGiveTime = g_app.getOuterFactoryPtr()->GetCustomTimeTick(itTField->colValue);
            }
        }
        ROLLLOG_DEBUG << "get friends give info, friend_uid: " << lFriendUid << ", iGiveTime: " << iGiveTime << endl;
        if ((lFriendUid > 0) && (iGiveTime > 0) && (lFriendUid == friend_uid))
        {
            give_time = iGiveTime;
            break;
        }
    }

    ROLLLOG_DEBUG << "====================== uid: " << uid << ", friend_uid: " << friend_uid << ", give_time: " << give_time << endl;

    __CATCH__
    FUNC_EXIT("", iRet);
    return iRet;
}

//USER_ACCOUNT = 20,     //#tbl_user_account
int Processor::selectUserAccount(const userinfo::GetUserReq &req, userinfo::GetUserResp &resp)
{
    if (req.uid <= 0)
    {
        ROLLLOG_ERROR << "invalid params, uid: " << req.uid << endl;
        resp.resultCode = -1;
        return -1;
    }

    TReadDataReq dataReq;
    dataReq.resetDefautlt();
    dataReq.keyName = I2S(E_REDIS_TYPE_HASH) + ":" + I2S(USER_ACCOUNT) + ":" + L2S(req.uid);
    dataReq.operateType = E_REDIS_READ;
    dataReq.clusterInfo.resetDefautlt();
    dataReq.clusterInfo.busiType = E_REDIS_PROPERTY;
    dataReq.clusterInfo.frageFactorType = E_FRAGE_FACTOR_STRING;
    dataReq.clusterInfo.frageFactor = tars::hash<string>()(L2S(req.uid));

    vector<TField> fields;
    TField tfield;
    tfield.colArithType = E_NONE;
    tfield.colName = "uid";
    fields.push_back(tfield);
    tfield.colName = "username";
    fields.push_back(tfield);
    tfield.colName = "password";
    fields.push_back(tfield);
    tfield.colName = "safes_password";
    fields.push_back(tfield);
    tfield.colName = "reg_type";
    fields.push_back(tfield);
    tfield.colName = "reg_time";
    fields.push_back(tfield);
    tfield.colName = "reg_ip";
    fields.push_back(tfield);
    tfield.colName = "reg_device_no";
    fields.push_back(tfield);
    tfield.colName = "is_robot";

    fields.push_back(tfield);
    tfield.colName = "agcid";
    fields.push_back(tfield);
    tfield.colName = "disabled";
    fields.push_back(tfield);
    tfield.colName = "device_id";
    fields.push_back(tfield);
    tfield.colName = "device_type";
    fields.push_back(tfield);
    tfield.colName = "platform";
    fields.push_back(tfield);
    tfield.colName = "channel_id";
    fields.push_back(tfield);
    tfield.colName = "area_id";
    fields.push_back(tfield);
    tfield.colName = "is_forbidden";
    fields.push_back(tfield);
    tfield.colName = "forbidden_time";
    fields.push_back(tfield);
    tfield.colName = "bindChannelId";
    fields.push_back(tfield);
    tfield.colName = "bindOpenId";
    fields.push_back(tfield);
    tfield.colName = "isinwhitelist";
    fields.push_back(tfield);
    tfield.colName = "whitelisttime";
    fields.push_back(tfield);
    dataReq.fields = fields;

    dataproxy::TReadDataRsp dataRsp;
    int iRet = g_app.getOuterFactoryPtr()->getDBAgentServantPrx(req.uid)->redisRead(dataReq, dataRsp);
    if ((iRet != 0) || (dataRsp.iResult != 0))
    {
        ROLLLOG_ERROR << "get user-account failed, uid: " << req.uid << ", iResult: " << dataRsp.iResult << endl;
        resp.resultCode = -1;
        return -2;
    }

    if (dataRsp.fields.empty())
    {
        ROLLLOG_ERROR << "uid:" << req.uid << " not exist in tb_useraccount!" << endl;
        resp.resultCode = -1;
        return -3;
    }

    for (auto it = dataRsp.fields.begin(); it != dataRsp.fields.end(); ++it)
    {
        for (auto itfields = it->begin(); itfields != it->end(); ++itfields)
        {
            if (itfields->colName == "username")
            {
                resp.userName = itfields->colValue;
            }
            else if (itfields->colName == "device_id")
            {
                resp.deviceID = itfields->colValue;
            }
            else if (itfields->colName == "device_type")
            {
                resp.deviceType = itfields->colValue;
            }
            else if (itfields->colName == "platform")
            {
                resp.platform = (userinfo::E_Platform_Type)S2I(itfields->colValue);
            }
            else if (itfields->colName == "channel_id")
            {
                resp.channnelID = (userinfo::E_Channel_ID)S2I(itfields->colValue);
            }
            else if (itfields->colName == "area_id")
            {
                resp.areaID = S2I(itfields->colValue);
            }
            else if (itfields->colName == "is_robot")
            {
                resp.isRobot = S2I(itfields->colValue);
            }
            else if (itfields->colName == "reg_time")
            {
                resp.regTime = g_app.getOuterFactoryPtr()->GetCustomTimeTick(itfields->colValue);
            }
            else if (itfields->colName == "bindChannelId")
            {
                resp.bindChannelId = S2I(itfields->colValue);
            }
            else if (itfields->colName == "bindOpenId")
            {
                resp.bindOpenId = itfields->colValue;
            }
            else if (itfields->colName == "reg_type")
            {
                resp.regType = S2I(itfields->colValue);
            }
            else if (itfields->colName == "isinwhitelist")
            {
                resp.isinwhitelist = S2I(itfields->colValue);
            }
            else if (itfields->colName == "whitelisttime")
            {
                resp.whitelisttime = g_app.getOuterFactoryPtr()->GetCustomTimeTick(itfields->colValue);
            }
        }
    }

    ROLLLOG_DEBUG << "get user succ, userinfo::GetUserReq:" << printTars(req) << ", userinfo::GetUserResp:" << printTars(resp) << endl;
    resp.resultCode = 0;
    return 0;
}
