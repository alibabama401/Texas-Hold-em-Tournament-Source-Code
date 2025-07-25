#include "xtime_userface.h"

SetSoTimer_t pSetSoTimer;
SetCbTimer_t pSetCbTimer;
KillSoTimer_t pKillSoTimer;
GetTimerRemaining_t pGetTimerRemaining;

extern "C"
{
    void initSoTimer(SetSoTimer_t setSoFunc, SetCbTimer_t setCbFunc, KillSoTimer_t killFunc, GetTimerRemaining_t getTimerRemainingFunc)
    {
        pSetSoTimer = setSoFunc;
        pSetCbTimer = setCbFunc;
        pKillSoTimer = killFunc;
        pGetTimerRemaining = getTimerRemainingFunc;
    }
}

///////////////////////////////////////////////////////////////////////////
typedef int (*soTickTockFunc)(int taskNum);
typedef soTickTockFunc (*getSoTickTockFunc)(int taskNum);

int soTickTock(int taskNum)
{
    return XTimer::onTickTock(taskNum);
}

extern "C"
{
    soTickTockFunc getSoTickTock()
    {
        return soTickTock;
    }
}

///////////////////////////////////////////////////////////////
//handler 管理方式的实现
int IXTimeHandler::loopDo(int user_cmd, xtime_t interval)
{
    xid_t ret = 0;

    if (pSetSoTimer)
    {
        ret = (*pSetSoTimer)(this, user_cmd, interval, interval);
    }

    _mmap_cmd_xid.insert( std::make_pair(user_cmd, ret));
    return ret;
}

xid_t IXTimeHandler::delayDo(int user_cmd, xtime_t delaySec)
{
    xid_t ret = 0;
    if (pSetSoTimer)
    {
        ret = (*pSetSoTimer)(this, user_cmd, delaySec, 0);
    }

    _mmap_cmd_xid.insert(std::make_pair(user_cmd, ret));
    return ret;
}

xtime_t IXTimeHandler::remaining(int user_cmd)
{
    xtime_t ret = 0;
    auto itLower = _mmap_cmd_xid.lower_bound(user_cmd);
    if (itLower != _mmap_cmd_xid.end())
    {
        auto timerId = itLower->second;
        if (pGetTimerRemaining)
        {
            ret = (*pGetTimerRemaining)(timerId);
        }
    }

    return ret;
}

int IXTimeHandler::cancelDo(int user_cmd, int start, int end)
{
    auto itLower = _mmap_cmd_xid.lower_bound(user_cmd);
    auto itUpper = _mmap_cmd_xid.upper_bound(user_cmd);
    if (itLower == itUpper)
    {
        return -1;
    }

    int ret = 0;
    int direction = end - start;
    if (direction == 0 && start == 0)
    {
        for (; itLower != itUpper;)
        {
            if (pKillSoTimer)
            {
                ret = (*pKillSoTimer)(itLower->second);
            }

            _mmap_cmd_xid.erase(itLower++);
        }
    }
    else if (start != 0 && direction == 0 )
    {
        //删除正向 第n新的定时器
    }
    else if ( direction > 0 )
    {
        //正向删除 最新到最早
    }
    else // direction < 0
    {
        //逆向删除 最早到最新
    }

    return ret;
}

int IXTimeHandler::clearDoAll()
{
    for (auto &kv : _mmap_cmd_xid )
    {
        if (pKillSoTimer)
        {
            (*pKillSoTimer)(kv.second);
        }
    }

    _mmap_cmd_xid.clear();
    return 0;
}

int IXTimeHandler::cancelDo(int cmd, xid_t xid)
{
    auto it = _mmap_cmd_xid.begin();
    while (it != _mmap_cmd_xid.end())
    {
        if (it->first == cmd && it->second == xid)
        {
            if (pKillSoTimer)
            {
                (*pKillSoTimer)(it->second);
            }

            _mmap_cmd_xid.erase(it++);
        }
    }

    return 0;
}

