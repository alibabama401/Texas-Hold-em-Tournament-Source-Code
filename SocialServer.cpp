#include "SocialServer.h"
#include "SocialServantImp.h"
#include "LogComm.h"

//
using namespace std;

//
SocialServer g_app;

/////////////////////////////////////////////////////////////////
void
SocialServer::initialize()
{
    //initialize application here:
    //...

    ///
    addServant<SocialServantImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".SocialServantObj");

    //初始化外部接口对象
    initOuterFactory();

    // 注册动态加载命令
    TARS_ADD_ADMIN_CMD_NORMAL("reload", SocialServer::reloadSvrConfig);
}
/////////////////////////////////////////////////////////////////
void
SocialServer::destroyApp()
{
    //destroy application here:
    //...
}

/*
* 配置变更，重新加载配置
*/
bool SocialServer::reloadSvrConfig(const string &command, const string &params, string &result)
{
    try
    {
        //加载配置
        getOuterFactoryPtr()->load();

        result = "reload server config success.";

        LOG_DEBUG << "reloadSvrConfig: " << result << endl;

        return true;
    }
    catch (TC_Exception const &e)
    {
        result = string("catch tc exception: ") + e.what();
    }
    catch (std::exception const &e)
    {
        result = string("catch std exception: ") + e.what();
    }
    catch (...)
    {
        result = "catch unknown exception.";
    }

    result += "\n fail, please check it.";

    LOG_DEBUG << "reloadSvrConfig: " << result << endl;

    return true;
}

/**
* 初始化外部接口对象
**/
int SocialServer::initOuterFactory()
{
    _pOuter = new OuterFactoryImp();

    return 0;
}

/////////////////////////////////////////////////////////////////
int
main(int argc, char *argv[])
{
    try
    {
        g_app.main(argc, argv);
        g_app.waitForShutdown();
    }
    catch (std::exception &e)
    {
        cerr << "std::exception:" << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }

    return -1;
}
/////////////////////////////////////////////////////////////////
