#ifndef __XTIMER_USERFACE_H__
#define __XTIMER_USERFACE_H__

#include <iostream>
#include <string>
#include <map>
#include <tuple>
#include "xtimer.h"

/////////////////////////////////////////////////////////////////////////////////
//尽量不直接使用XTimer名字空间的接口

namespace XTimer
{
    //一次性定时器
    template<typename UserDataBodyT>
    inline xid_t setTimer(xtime_t lifetime, std::function<int(TimerParam &)> cb, int timer_key, UserDataBodyT *paramBody)
    {
        return XTimer::setTimer(lifetime, cb, TimerParam(0, timer_key, static_cast<void *>(paramBody), XTimer::Destructor<UserDataBodyT>));
    }

    //周期定时器
    template<typename UserDataBodyT>
    inline xid_t setTimer(std::function<int(TimerParam &)> cb, int timer_key, UserDataBodyT *paramBody, xtime_t delaytime, xtime_t interval)
    {
        return XTimer::setTimer(delaytime, cb, TimerParam(interval, timer_key, static_cast<void *>(paramBody), XTimer::Destructor<UserDataBodyT>));
    }
}

using XTimer::xid_t;
using XTimer::xtime_t;
using XTimer::TimerParam;
using XTimer::make_body;
using XTimer::DestructorFunc_t;

//////////////////////////////////////////////////////////////////////////////
//**********************************************
// handler方式所需继承的基类(推荐使用)
// 说明： 适用于有状态的具有"根类"的业务模型
//        业务定时器的管理均由"根类"管理，管理方法通过继承此类提供
//
//
//
class IXTimeHandler
{
    friend xid_t setTimer(IXTimeHandler *handler, int user_cmd, xtime_t delaytime, xtime_t interval = 0);

public:
    ///
    // Function:超时处理接口
    // Describe:需要设定闹钟的实例需实现此接口
    // Return:
    // Param
    ///
    virtual int onTimeout(int user_cmd) = 0;

    //
    int loopDo(int user_cmd, xtime_t interval);

    //
    xid_t delayDo(int user_cmd, xtime_t delaySec);

    ///
    // Function:获取到期剩余时间
    // Describe:
    // Return:
    // Param
    ///
    xtime_t remaining(int user_cmd);

    ///
    // Function:删除定时器
    // Describe:后2个参数的特殊说明
    //          定义 从最新时钟 指向 最旧时钟 方向为正向,对应的start 应当大于 end
    //          反之 如果 start 小于 end 则定义为 反向  ，从最旧时钟指向最新时钟
    //          起始序号 为 1
    //          序号区间为后闭前开区间
    //          0,0 指所有当前命令字的定时器
    //          0,1 指最新添加的当前命令定时器
    //          1,0 指最新添加的当前命令定时器
    //          1,2 指第二新添加的当前命令定时器
    //          2,1 指第二早添加的当前命令定时器
    //
    // Return int:预留
    // Param start 起始位置序号
    // Param end   结束位置序号
    ///
    int cancelDo(int user_cmd, int start, int end);

    //
    inline int cancelDo(int user_cmd)
    {
        return cancelDo(user_cmd, 0, 0);
    }

    //
    int clearDoAll();

protected:
    //
    int cancelDo(int cmd, xid_t xid);
    //
    std::multimap<int, xid_t> _mmap_cmd_xid;
};


///
//Function:  handler载体结构
//此结构用于handler内部,提供时钟对handler的参数管理和转化
///
struct HandlerWayBody
{
    HandlerWayBody(IXTimeHandler *handler, int user_cmd): handler(handler), user_cmd(user_cmd), xid(-2)
    {

    }

    IXTimeHandler *handler;
    int user_cmd;
    xid_t xid;
};



///
// Function:handler方式接口
// Describe:此接口用户handler类内部，仅限于需要重新定制自己的handler基类时才需要使用
///
inline xid_t setTimer(IXTimeHandler *handler, int user_cmd, xtime_t delaytime, xtime_t interval)
{
    auto cb = [](TimerParam & param)->int
    {
        auto body = static_cast<HandlerWayBody const *>(param.getBody());
        return body->handler->onTimeout(body->user_cmd);
    };

    auto pHbody = make_body<HandlerWayBody>(HandlerWayBody(handler, user_cmd));
    pHbody->xid = XTimer::setTimer<HandlerWayBody>(cb, -1, pHbody, delaytime, interval);
    return pHbody->xid;
}


////////////////////////////////////////////////////////////////////////////////////
//交互用类型
//
//so宿主交互(handlerWay)
typedef xid_t(*SetSoTimer_t)(IXTimeHandler *, int, xtime_t, xtime_t);
//so宿主交互(callbackWay)
typedef xid_t(*SetCbTimer_t)(void *, std::function<int(TimerParam &)>, DestructorFunc_t, int, xtime_t, xtime_t);
typedef xid_t(*KillSoTimer_t)(xid_t);
typedef xtime_t (*GetTimerRemaining_t)(xid_t);
typedef void (*initSoTimer_t)(SetSoTimer_t, SetCbTimer_t, KillSoTimer_t, GetTimerRemaining_t);

//////////////////////////////////////////////////////
//动态库的调用时钟的函数指针 请忽略
extern SetSoTimer_t pSetSoTimer;
extern SetCbTimer_t pSetCbTimer;
extern KillSoTimer_t pKillSoTimer;
extern GetTimerRemaining_t pGetTimerRemaining;

//////////////////////////////////////////////////////
//空悬参数方式调用接口
// Function:空悬参数方式调用接口
// Describe:不推荐直接使用,动态库内应该按照xtime4lib的示例方式使用
//                         Host内应该按照xtime4host的示例方式使用
// Return:
// Param
///
template<typename T>
inline xid_t SetSoTimer(T &&user_body, std::function<int(TimerParam &)> cb, int timerKey, xtime_t delaytime, xtime_t interval)
{
    if (!pSetCbTimer)
        return 0;

    return (*pSetCbTimer)(make_body<T>(user_body), cb, XTimer::Destructor<T>, timerKey, delaytime, interval);
}

///////////////////////////////////////////////////////
// Function:停止闹钟
///
inline xid_t killTimer(xid_t timerId)
{
    if (!pKillSoTimer)
        return 0;

    return (*pKillSoTimer)(timerId);
}


///////////////////////////////////////////////////////////////
//定时器管理集合类模板
//
//IndexT,定时器集合索引类
//CmdT,命令字类
//XidContainer 某索引 的时钟集合容器
//StdContainer 索引key到集合容器的映射
//
template < typename IndexT,
           typename CmdT,
           typename XidContainer = std::map<CmdT, xid_t>,
           typename StdContainer = std::map<IndexT, XidContainer> >
class Aggregate
{
protected:
    //
    StdContainer manAggr;

public:
    //
    void addXid(IndexT idx, CmdT cmd, xid_t xid)
    {
        auto it = manAggr.find(idx);
        if (it == manAggr.end())
        {
            XidContainer mm_cmd_xids;
            mm_cmd_xids.insert(make_pair(cmd, xid));
            manAggr.insert(make_pair(idx, mm_cmd_xids));
        }
        else
        {
            auto itt = it->second.find(cmd);
            if(itt != it->second.end())
            {
                kill(idx, cmd);
            }
            it->second.insert(make_pair(cmd, xid));
            PERFSTATS("@bug timer addXid. idx: "<< idx<< ", cmd: "<< cmd << ", timer size: "<< it->second.size());
        }
    }
    //
    void kill(IndexT idx, CmdT cmd)
    {
        auto it = manAggr.find(idx);
        if (it == manAggr.end())
            return;

        auto& cx = it->second;
        for (auto xiter = cx.begin(); xiter != cx.end();)
        {
            if (xiter->first == cmd)
            {
                if (pKillSoTimer)
                {
                    (*pKillSoTimer)(xiter->second);
                }

                cx.erase(xiter++);
                PERFSTATS("@bug timer kill. idx: "<< idx<< ", cmd: "<< cmd << ", timer size: "<< cx.size());
            }
            else
            {
                ++xiter;
            }
        }
    }
    //
    void kill(IndexT idx)
    {
        auto it = manAggr.find(idx);
        if (it == manAggr.end())
            return;

        auto& cx = it->second;
        for (auto xiter = cx.begin(); xiter != cx.end();)
        {
            if (pKillSoTimer)
            {
                (*pKillSoTimer)(xiter->second);
            }

            cx.erase(xiter++);
        }
        PERFSTATS("@bug timer kill all. idx: "<< idx << ", timer size: "<< cx.size());
    }
    //
    int remaining(IndexT idx, CmdT cmd)
    {
        auto it = manAggr.find(idx);
        if (it == manAggr.end())
            return 0;

        auto cx = it->second;
        auto itLower = cx.lower_bound(cmd);
        if (itLower == cx.end())
            return 0;

        if (!pGetTimerRemaining)
            return 0;

        return (*pGetTimerRemaining)(itLower->second);
    }
};

//经典xid管理类
typedef Aggregate<std::string, int> ClassicAggrImpl;
//经典xid管理类单例
typedef XTimer::Singleton<ClassicAggrImpl> Aggr;


///////////////////////////////////////////////////////////////
//**************************
//简化空悬操作(推荐使用)
//

#define DELAYCALL_BEGIN(M_delay,M_interval,M_str,M_key,...) \
auto userStruct = make_tuple(M_str, ## __VA_ARGS__);\
auto cb = [](TimerParam & param)\
{\
    void* body = const_cast<void *>(param.getBody());\
    decltype(userStruct) & x = *static_cast<decltype(userStruct)*>(body);


#define GET(X) std::get<X>(x)


#define DELAYCALL_END \
Aggr::getInstance()->kill(std::get<0>(X),param.getKey());\
return 0;\
}\
xid_t xid = SetSoTimer<decltype(Bt)>(userStruct,cb,M_delay,M_interval);\
Aggr::getInstance()->addXid(M_str,M_key,xid);


#define KILL_TIMER(str,key) \
    Aggr::getInstance()->kill(str,key)

#define KILL_ALL_TIMER(str)\
    Aggr::getInstance()->kill(str)

#endif /*__XTIMER_USERFACE_H__*/
