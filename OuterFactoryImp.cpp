#include <sstream>
#include <random>
#include "OuterFactoryImp.h"
#include "LogComm.h"
#include "SocialServer.h"


/**
 *
*/
OuterFactoryImp::OuterFactoryImp(): _pFileConf(NULL)
{
    createAllObject();
}

OuterFactoryImp::~OuterFactoryImp()
{
    deleteAllObject();
}

void OuterFactoryImp::deleteAllObject()
{
    if (_pFileConf)
    {
        delete _pFileConf;
        _pFileConf = NULL;
    }
}

void OuterFactoryImp::createAllObject()
{
    try
    {
        deleteAllObject();

        //本地配置文件
        _pFileConf = new tars::TC_Config();

        //tars代理Factory,访问其他tars接口时使用
        _pProxyFactory = new OuterProxyFactory();
        LOG_DEBUG << "init proxy factory succ." << endl;

        //加载配置
        load();
    }
    catch (TC_Exception &ex)
    {
        LOG->error() << ex.what() << endl;
        throw;
    }
    catch (exception &e)
    {
        LOG->error() << e.what() << endl;
        throw;
    }
    catch (...)
    {
        LOG->error() << "unknown exception." << endl;
        throw;
    }

    return;
}

//读取所有配置
void OuterFactoryImp::load()
{
    __TRY__

    //拉取远程配置
    g_app.addConfig(ServerConfig::ServerName + ".conf");

    WriteLocker lock(m_rwlock);

    //
    _pFileConf->parseFile(ServerConfig::BasePath + ServerConfig::ServerName + ".conf");
    LOG_DEBUG << "init config file succ:" << ServerConfig::BasePath + ServerConfig::ServerName + ".conf" << endl;

    //代理配置
    readPrxConfig();
    printPrxConfig();

    // //最大好友数量
    // readMaxFriendsCount();
    // printMaxFriendsCount();

    //加载通用配置
    readGeneralConfigResp();
    printGeneralConfigResp();

    __CATCH__
}

//代理配置
void OuterFactoryImp::readPrxConfig()
{
    _ConfigServantObj = (*_pFileConf).get("/Main/Interface/ConfigServer<ProxyObj>", "");
    _DBAgentServantObj = (*_pFileConf).get("/Main/Interface/DBAgentServer<ProxyObj>", "");
    _PushServantObj = (*_pFileConf).get("/Main/Interface/PushServer<ProxyObj>", "");
    _HallServantObj = (*_pFileConf).get("/Main/Interface/HallServer<ProxyObj>", "");
}

//
void OuterFactoryImp::printPrxConfig()
{
    ROLLLOG_DEBUG << "_ConfigServantObj ProxyObj:" << _ConfigServantObj << endl;
    ROLLLOG_DEBUG << "_DBAgentServantObj ProxyObj:" << _DBAgentServantObj << endl;
    ROLLLOG_DEBUG << "_PushServantObj ProxyObj:" << _PushServantObj << endl;
    ROLLLOG_DEBUG << "_HallServantObj ProxyObj:" << _HallServantObj << endl;
}

//最大好友数量
// void OuterFactoryImp::readMaxFriendsCount()
// {
//     maxFriendsCount = TC_Common::strto<int>((*_pFileConf).get("/Main<MaxFriendsCount>", "0"));
// }

// void OuterFactoryImp::printMaxFriendsCount()
// {
//     ROLLLOG_DEBUG << "maxFriendsCount : " << maxFriendsCount << endl;
// }

//最大好友数量
int OuterFactoryImp::getMaxFriendsCount()
{
    int type = config::E_GENERAL_TYPE_FRIENDS_MAXNUM;//通用配置-添加好友数量上限
    auto iter = listGeneralConfigResp.data.find(type);
    if ((iter == listGeneralConfigResp.data.end()) || ((int)iter->second.size() != 1))
    {
        ROLLLOG_ERROR << "getMaxFriendsCount failed, type: " << type << ", size: " << iter->second.size() << endl;
        return 0;
    }
    auto itCfg = iter->second.begin();
    if (itCfg->second.value < 0)
    {
        ROLLLOG_ERROR << "listGeneralConfigResp value error, type: " << type << ", value: " << itCfg->second.value << endl;
        return 0;
    }

    ROLLLOG_DEBUG << "listGeneralConfigResp MaxFriendsCount:" << itCfg->second.value << ", type:" << type << endl;
    return itCfg->second.value;
}

//加载通用配置
void OuterFactoryImp::readGeneralConfigResp()
{
    getConfigServantPrx()->ListGeneralConfig(listGeneralConfigResp);
}

void OuterFactoryImp::printGeneralConfigResp()
{
    ROLLLOG_DEBUG << "listGeneralConfigResp: " << printTars(listGeneralConfigResp) << endl;
}

//格式化时间
string OuterFactoryImp::GetTLogTimeFormat()
{
    string sFormat("%Y-%m-%d %H:%M:%S");
    time_t t = time(NULL);
    struct tm *pTm = localtime(&t);
    if (pTm == NULL)
    {
        return "";
    }

    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);
    return string(sTimeString);
}

//格式化自定义时间
string OuterFactoryImp::GetCustomTimeFormat(int time)
{
    string sFormat("%Y-%m-%d %H:%M:%S");
    time_t t = time_t(time);
    auto pTm = localtime(&t);
    if (pTm == NULL)
    {
        return "";
    }

    char sTimeString[255] = "\0";
    strftime(sTimeString, sizeof(sTimeString), sFormat.c_str(), pTm);
    return string(sTimeString);
}

//获取自定义秒数
int OuterFactoryImp::GetCustomTimeTick(const string &str)
{
    if(str.empty())
    {
        return 0;
    }

    //
    struct tm tm_time;

    string sFormat("%Y-%m-%d %H:%M:%S");

    strptime(str.c_str(), sFormat.c_str(), &tm_time);

    return mktime(&tm_time);
}

//格式化自定义年月日
int OuterFactoryImp::GetCustomDateFormat(int time)
{
    string sFormat("%Y%m%d");
    time_t t = time_t(time);
    struct tm *pTm = localtime(&t);
    if (pTm == NULL)
    {
        return 0;
    }

    char sDateString[10] = "\0";
    strftime(sDateString, sizeof(sDateString), sFormat.c_str(), pTm);

    return S2I(sDateString);
}

//推送添加好友消息
void OuterFactoryImp::asyncRequest2PushAddFriend(const long uid, const push::PushMsgReq &msg)
{
    getPushServantPrx(uid)->async_pushMsg(NULL, msg);
}

//拆分字符串成整形
int OuterFactoryImp::splitInt(string szSrc, vector<int> &vecInt)
{
    split_int(szSrc, "[ \t]*\\|[ \t]*", vecInt);
    return 0;
}

//随机范围数
int OuterFactoryImp::nnrand(int max, int min)
{
    std::random_device rd;
    srand(rd());
    return min + rand() % (max - min + 1);
}

//游戏配置服务代理
const ConfigServantPrx OuterFactoryImp::getConfigServantPrx()
{
    if (!_ConfigServerPrx)
    {
        _ConfigServerPrx = Application::getCommunicator()->stringToProxy<config::ConfigServantPrx>(_ConfigServantObj);
        ROLLLOG_DEBUG << "Init _ConfigServantObj succ, _ConfigServantObj:" << _ConfigServantObj << endl;
    }

    return _ConfigServerPrx;
}

//数据库代理服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const long uid)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj:" << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(uid);
    }

    return NULL;
}

//数据库代理服务代理
const DBAgentServantPrx OuterFactoryImp::getDBAgentServantPrx(const string key)
{
    if (!_DBAgentServerPrx)
    {
        _DBAgentServerPrx = Application::getCommunicator()->stringToProxy<dbagent::DBAgentServantPrx>(_DBAgentServantObj);
        ROLLLOG_DEBUG << "Init _DBAgentServantObj succ, _DBAgentServantObj:" << _DBAgentServantObj << endl;
    }

    if (_DBAgentServerPrx)
    {
        return _DBAgentServerPrx->tars_hash(tars::hash<string>()(key));
    }

    return NULL;
}

//PushServer代理
const PushServantPrx OuterFactoryImp::getPushServantPrx(const long uid)
{
    if (!_PushServerPrx)
    {
        _PushServerPrx = Application::getCommunicator()->stringToProxy<push::PushServantPrx>(_PushServantObj);
        ROLLLOG_DEBUG << "Init _PushServantObj succ, _PushServantObj:" << _PushServantObj << endl;
    }

    if (_PushServerPrx)
    {
        return _PushServerPrx->tars_hash(uid);
    }

    return NULL;
}

//HallServantPrx代理
const HallServantPrx OuterFactoryImp::getHallServantPrx(const long uid)
{
    if (!_HallServantPrx)
    {
        _HallServantPrx = Application::getCommunicator()->stringToProxy<hall::HallServantPrx>(_HallServantObj);
        ROLLLOG_DEBUG << "Init _HallServantObj succ, _HallServantObj:" << _HallServantObj << endl;
    }

    if (_HallServantPrx)
    {
        return _HallServantPrx->tars_hash(uid);
    }

    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
