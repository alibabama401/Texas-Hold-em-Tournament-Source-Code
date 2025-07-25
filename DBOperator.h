#ifndef _DB_OPERATOR_H_
#define _DB_OPERATOR_H_

#include <string>
#include <map>

#include <util/tc_config.h>
#include <util/tc_mysql.h>
#include <util/tc_singleton.h>
#include <util/tc_autoptr.h>
#include <wbl/pthread_util.h>

//协议
#include "DBConfigProto.h"
#include "RobotConfig.h"

using namespace std;
using namespace tars;
using namespace ai;
using namespace wbl;

/**
*
* DB操作类，用于读取配置
*/
class CDBOperator : public TC_HandleBase
{
public:
    CDBOperator();
    ~CDBOperator();

public:
    //
    int init();
    //
    int init(const TC_DBConf &dbConf);
    //
    int init(const string &dbhost, const string &dbuser, const string &dbpass, const string &dbname, const string &charset, const string &dbport);

public:
    //加载配置数据
    int loadConfig();
    //加载机器人配置信息
    int loadRobotConfig();
    //打印机器人配置信息
    int printRobotConfig();
    //打印机器人配置信息
    int printRobotConfigMap();
    //加载机器人充值配置信息
    int loadRobotRechargeConfig();
    //打印机器人充值配置信息
    int printRobotRechargeConfig();
    //获取取机器人配置
    const vector<TRobotConf> &getRobotData();
    //获取机器人数据
    const map<int, TRobotConf> &getRobotDataMap();
    //获取机器人充值
    const map<int, vector<TRobotRechargeConfig> > &getRobotRechargeData();

private:
    //机器人配置数据
    vector<TRobotConf> vecRobotData;
    //机器人配置数据
    map<int, TRobotConf> mapRobotData;
    //机器人充值数据
    map<int, vector<TRobotRechargeConfig>> mapRobotRechargeData;

private:
    //读写锁，防止数据脏读
    wbl::ReadWriteLocker m_rwlock;
    //mysql操作对象
    TC_Mysql m_mysqlObj;
};

//singleton
typedef TC_Singleton<CDBOperator, CreateStatic, DefaultLifetime> DBOperatorSingleton;

//ptr
typedef TC_AutoPtr<CDBOperator> CDBOperatorPtr;

#endif


