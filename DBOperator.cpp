#include <sstream>
#include <util/tc_common.h>
#include "DBOperator.h"
#include "globe.h"
#include "LogComm.h"

//
using namespace wbl;

CDBOperator::CDBOperator()
{

}

CDBOperator::~CDBOperator()
{

}

//初始化
int CDBOperator::init()
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        //for test
        map<string, string> mpParam;
        mpParam["dbhost"]  = "localhost";
        mpParam["dbuser"]  = "tars";
        mpParam["dbpass"]  = "tars2015";
        mpParam["dbname"]  = "config";
        mpParam["charset"] = "utf8";
        mpParam["dbport"]  = "3306";

        TC_DBConf dbConf;
        dbConf.loadFromMap(mpParam);
        m_mysqlObj.init(dbConf);
    }
    catch (exception &e)
    {
        iRet = -1;
        ROLLLOG_ERROR << "Catch exception: " << e.what() << endl;
    }
    catch (...)
    {
        iRet = -2;
        ROLLLOG_ERROR << "Catch unknown exception." << endl;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//初始化
int CDBOperator::init(const string &dbhost, const string &dbuser, const string &dbpass, const string &dbname, const string &charset, const string &dbport)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        map<string, string> mpParam;
        mpParam["dbhost"]  = dbhost;
        mpParam["dbuser"]  = dbuser;
        mpParam["dbpass"]  = dbpass;
        mpParam["dbname"]  = dbname;
        mpParam["charset"] = charset;
        mpParam["dbport"]  = dbport;

        TC_DBConf dbConf;
        dbConf.loadFromMap(mpParam);
        m_mysqlObj.init(dbConf);
    }
    catch (exception &e)
    {
        iRet = -1;
        ROLLLOG_ERROR << "Catch exception: " << e.what() << endl;
    }
    catch (...)
    {
        iRet = -2;
        ROLLLOG_ERROR << "Catch unknown exception." << endl;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//初始化
int CDBOperator::init(const TC_DBConf &dbConf)
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        m_mysqlObj.init(dbConf);
    }
    catch (exception &e)
    {
        iRet = -1;
        ROLLLOG_ERROR << "Catch exception: " << e.what() << endl;
    }
    catch (...)
    {
        iRet = -2;
        ROLLLOG_ERROR << "Catch unknown exception." << endl;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//加载配置数据
int CDBOperator::loadConfig()
{
    FUNC_ENTRY("");
    int iRet = 0;

    ////
    WriteLocker lock(m_rwlock);

    //加载机器人配置信息
    iRet = loadRobotConfig();
    if (iRet != 0)
    {
        ROLLLOG_ERROR << "load robot config fail, ret: " << iRet << endl;
        return -1;
    }

    //打印机器人配置信息
    printRobotConfig();
    printRobotConfigMap();

    //加载机器人充值配置信息
    iRet = loadRobotRechargeConfig();
    if (iRet != 0)
    {
        ROLLLOG_ERROR << "load robot recharge config fail, ret: " << iRet << endl;
        return -2;
    }

    //打印机器人充值配置信息
    printRobotRechargeConfig();

    FUNC_EXIT("", iRet);
    return iRet;
}

//加载机器人配置信息
int CDBOperator::loadRobotConfig()
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        string strSQL = "select room_id, game_id, batch_id, robot_count, service_type," \
                        " UNIX_TIMESTAMP(entry_time) as entry_time, UNIX_TIMESTAMP(leave_time) as leave_time,"  \
                        " min_coins, max_coins, entry_min_interval,"    \
                        " entry_max_interval, min_round, max_round, min_play_time, max_play_time, min_winning_ratio, max_winning_ratio, description" \
                        " from robot_config where status = 1";
        TC_Mysql::MysqlData res = m_mysqlObj.queryRecord(strSQL);
        ROLLLOG_DEBUG << "Execute SQL: [" << strSQL << "], return " << res.size() << " records." << endl;
        if (res.size() <= 0)
        {
            ROLLLOG_WARN << " no data." << endl;
            return 0;
        }

        vecRobotData.clear();
        for (size_t i = 0; i < res.size(); ++i)
        {
            TRobotConf robotConf;
            robotConf.iGameID = TC_Common::strto<int>(res[i]["game_id"]);
            robotConf.sRoomID = res[i]["room_id"];
            robotConf.iBatchID = TC_Common::strto<int>(res[i]["batch_id"]);
            robotConf.iRobotCount = TC_Common::strto<int>(res[i]["robot_count"]);
            robotConf.eServiceType = (Eum_Service_Type)(TC_Common::strto<int>(res[i]["service_type"]));
            robotConf.iEntryTime = TC_Common::strto<int>(res[i]["entry_time"]);
            robotConf.iLeaveTime = TC_Common::strto<int>(res[i]["leave_time"]);
            robotConf.iMinCoins = TC_Common::strto<tars::Int64>(res[i]["min_coins"]);
            robotConf.iMaxCoins = TC_Common::strto<tars::Int64>(res[i]["max_coins"]);
            robotConf.iEntryMinInterval = TC_Common::strto<int>(res[i]["entry_min_interval"]);
            robotConf.iEntryMaxInterval = TC_Common::strto<int>(res[i]["entry_max_interval"]);
            robotConf.iMinRound = TC_Common::strto<int>(res[i]["min_round"]);
            robotConf.iMaxRound = TC_Common::strto<int>(res[i]["max_round"]);
            robotConf.iMinPlayTime = TC_Common::strto<int>(res[i]["min_play_time"]);
            robotConf.iMaxPlayTime = TC_Common::strto<int>(res[i]["max_play_time"]);
            robotConf.iMinWinningRatio = TC_Common::strto<int>(res[i]["min_winning_ratio"]);
            robotConf.iMaxWinningRatio = TC_Common::strto<int>(res[i]["max_winning_ratio"]);
            robotConf.description = res[i]["description"];
            vecRobotData.push_back(robotConf);
            mapRobotData[robotConf.iBatchID] = robotConf;
        }
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_DEBUG << "select operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_DEBUG << "select operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//打印机器人配置信息
int CDBOperator::printRobotConfig()
{
    FUNC_ENTRY("");
    int iRet = 0;

    ostringstream os;
    os << "robot config count: " << vecRobotData.size() << endl;
    os << "robot config values: ";
    for (auto it = vecRobotData.begin(); it != vecRobotData.end(); ++it)
    {
        os << "iGameID: " << it->iGameID << ", sRoomID: " << it->sRoomID << ", iBatchID: " << it->iBatchID
           << ", iRobotCount: " << it->iRobotCount << ", eServiceType: " << it->eServiceType << ", iEntryTime: " << it->iEntryTime
           << ", iLeaveTime: " << it->iLeaveTime << ", iMinCoins: " << it->iMinCoins << ", iMaxCoins: " << it->iMaxCoins
           << ", iEntryMinInterval: " << it->iEntryMinInterval << ", iEntryMaxInterval: " << it->iEntryMaxInterval << ", iMinRound: " << it->iMinRound
           << ", iMaxRound: " << it->iMaxRound << ", iMinPlayTime: " << it->iMinPlayTime << ", iMaxPlayTime: " << it->iMaxPlayTime
           << ", iMinWinningRatio: " << it->iMinWinningRatio << ", iMaxWinningRatio: " << it->iMaxWinningRatio << ", description: " << it->description << endl;
    }

    //
    FDLOG_CONFIG_INFO << os.str() << endl;

    FUNC_EXIT("", iRet);
    return iRet;
}

//打印机器人配置信息
int CDBOperator::printRobotConfigMap()
{
    FUNC_ENTRY("");
    int iRet = 0;

    ostringstream os;
    os << "robot config map count: " << mapRobotData.size() << endl;
    os << "robot config map values: " << endl;
    for (auto it = mapRobotData.begin(); it != mapRobotData.end(); ++it)
    {
        os << "iGameID: " << it->second.iGameID << ", sRoomID: " << it->second.sRoomID << ", iBatchID: " << it->second.iBatchID
           << ", iRobotCount: " << it->second.iRobotCount << ", eServiceType: " << it->second.eServiceType << ", iEntryTime: " << it->second.iEntryTime
           << ", iLeaveTime: " << it->second.iLeaveTime << ", iMinCoins: " << it->second.iMinCoins << ", iMaxCoins: " << it->second.iMaxCoins
           << ", iEntryMinInterval: " << it->second.iEntryMinInterval << ", iEntryMaxInterval: " << it->second.iEntryMaxInterval << ", iMinRound: " << it->second.iMinRound
           << ", iMaxRound: " << it->second.iMaxRound << ", iMinPlayTime: " << it->second.iMinPlayTime << ", iMaxPlayTime: " << it->second.iMaxPlayTime
           << ", iMinWinningRatio: " << it->second.iMinWinningRatio << ", iMaxWinningRatio: " << it->second.iMaxWinningRatio << ", description: " << it->second.description << endl;
    }

    FDLOG_CONFIG_INFO << os.str() << endl;
    FUNC_EXIT("", iRet);
    return iRet;
}

//加载机器人充值配置信息
int CDBOperator::loadRobotRechargeConfig()
{
    FUNC_ENTRY("");
    int iRet = 0;

    try
    {
        string strSQL = "select batch_id, coin_type, min_coins, max_coins from robot_recharge where status = 1";
        TC_Mysql::MysqlData res = m_mysqlObj.queryRecord(strSQL);
        ROLLLOG_DEBUG << "Execute SQL: [" << strSQL << "], return " << res.size() << " records." << endl;
        if (res.size() <= 0)
        {
            ROLLLOG_WARN << " no data." << endl;
            return 0;
        }

        mapRobotRechargeData.clear();
        for (size_t i = 0; i < res.size(); ++i)
        {
            TRobotRechargeConfig rechargeConf;
            rechargeConf.iBatchID = TC_Common::strto<int>(res[i]["batch_id"]);
            rechargeConf.iCoinType = TC_Common::strto<int>(res[i]["coin_type"]);
            rechargeConf.iMinCoins = TC_Common::strto<tars::Int64>(res[i]["min_coins"]);
            rechargeConf.iMaxCoins = TC_Common::strto<tars::Int64>(res[i]["max_coins"]);

            auto it = mapRobotRechargeData.find(rechargeConf.iBatchID);
            if (it != mapRobotRechargeData.end())
            {
                it->second.push_back(rechargeConf);
            }
            else
            {
                vector<TRobotRechargeConfig> vecrechargeConf;
                vecrechargeConf.push_back(rechargeConf);
                mapRobotRechargeData[rechargeConf.iBatchID] = vecrechargeConf;
            }
        }
    }
    catch (TC_Mysql_Exception &e)
    {
        ROLLLOG_DEBUG << "select operator catch mysql exception: " << e.what() << endl;
        iRet = -1;
    }
    catch (...)
    {
        ROLLLOG_DEBUG << "select operator catch unknown exception." << endl;
        iRet = -2;
    }

    FUNC_EXIT("", iRet);
    return iRet;
}

//打印机器人充值配置信息
int CDBOperator::printRobotRechargeConfig()
{
    FUNC_ENTRY("");
    int iRet = 0;

    ostringstream os;
    os << "robot recharge config data count: " << mapRobotRechargeData.size() << endl;
    os << "robot recharge config data values: " << endl;
    for (auto it = mapRobotRechargeData.begin(); it != mapRobotRechargeData.end(); ++it)
    {
        for (auto itItem = it->second.begin(); itItem != it->second.end(); ++itItem)
        {
            os << "iBatchID: " << itItem->iBatchID
               << ", iCoinType: " << itItem->iCoinType
               << ", iMinCoins: " << itItem->iMinCoins
               << ", iMaxCoins: " << itItem->iMaxCoins << endl;
        }
    }

    FDLOG_CONFIG_INFO << os.str() << endl;

    FUNC_EXIT("", iRet);
    return iRet;
}

//
const vector<TRobotConf> &CDBOperator::getRobotData()
{
    wbl::ReadLocker lock(m_rwlock);
    return vecRobotData;
}

//
const map<int, TRobotConf> &CDBOperator::getRobotDataMap()
{
    wbl::ReadLocker lock(m_rwlock);
    return mapRobotData;
}

//
const map<int, vector<TRobotRechargeConfig>> &CDBOperator::getRobotRechargeData()
{
    wbl::ReadLocker lock(m_rwlock);
    return mapRobotRechargeData;
}

