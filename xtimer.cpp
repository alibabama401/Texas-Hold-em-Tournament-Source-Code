/***************************************************************************************
#  Copyright (c) 2018
#  All rights reserved
#
#  @author  :
#  @name    :
#  @file    :
#  @date    :
#  @describe:闹钟服务
#    1.需设定闹钟的主体类继Copyright 接口类,并需实现Timeout 接口
#    2.如果走秒由外部驱动,则使用getSoTickTock  获取 soTickTock 走秒函数
#    3.设计精度按秒计算,如果需要增加精度,更改外部晶振频率即可
#    4.接口类提供基本的DelayDo 和cancelDo 方法,使用需要在调用类内定义命令字，并在
#Timeout接口中实现分发。
#    5.使用delayDo时,对象
#    6.使用cancelDo时,如果定义有多次相同的命令字,可以指定顺序>>>>详见注释
#***************************************************************************************/

#include <iostream>
#include "xtimer.h"
#include "utils/tarslog.h"

#define IS_HANDLER_WAY  -1
#define INVALID_XID static_cast<xid_t>(-1)

namespace XTimer
{
    /////////////////////////////////////////////////////////////////////
    int XXTime::onTickTock(int taskNum)
    {
        //counter
        ++_currentTime;
        //task list
        std::map<xid_t, long> v;
        //event list
        auto iter = _triggerTimeID.begin();
        auto ends = _triggerTimeID.end();
        for ( ; iter != ends; )
        {
            if ((unsigned long)TNowMS() < iter->first)
            {
                iter++;
                continue;
            }

            xid_t xid = INVALID_XID;
            auto event = _idCallbackParam.find(iter->second);
            if (event != _idCallbackParam.end())
            {
                long ts = TNowMS();
                auto &param = event->second.second;
                auto cb = event->second.first;
                xid = event->first;

                try
                {
                    if (not param._killed)
                        cb(param);
                }
                catch (...)
                {
                    PERFSTATS("@scheduler event handling failed, taskId: " << param.getKey());
                }

                try
                {
                    if (param._killed)
                    {
                        param._interval = 0;
                    }

                    if (param._interval == 0)
                    {
                        if (param._destructor)
                            param._destructor(&(param._userStruct));
                    }
                }
                catch (...)
                {
                    PERFSTATS("@scheduler event cleaning failed, taskId: " << param.getKey());
                }

                if ((xid != INVALID_XID) && (param._interval == 0))
                    _idCallbackParam.erase(event);

                if ((xid != INVALID_XID) && (param._interval != 0))
                    _triggerTimeID.insert(std::make_pair(param._interval + TNowMS(), xid));

                v[param.getKey()] = (TNowMS() - ts);
            }

            _triggerTimeID.erase(iter++);
        }

        const long costTime = 3;
        for (auto iter = v.begin(); iter != v.end(); iter++)
        {
            if (((*iter).first > 9999999) || ((*iter).second < costTime))
                continue;

            PERFSTATS("@scheduler timer, taskId: " << (*iter).first << ", costTime:" << (*iter).second);
        }

        return 0;
    }

    xtime_t XXTime::getTimerRemaining(xid_t timerId)
    {
        auto iter = _idCallbackParam.find(timerId);
        if (iter == _idCallbackParam.end())
            return 0;

        TimerParam &param = iter->second.second;
        return (param._triggerTime - _currentTime);
    }

    xid_t XXTime::setTimer(xtime_t lifeTime, std::function<int(TimerParam &)> cb, TimerParam param)
    {
        // uint32_t executeTimes = lifeTime * getPeriodTimes();
        xid_t xid = generateTimerId();
        // xtime_t triggerTime = _currentTime + executeTimes;
        xtime_t triggerTime = TNowMS() + lifeTime * 1000;
        param._triggerTime = triggerTime;
        _triggerTimeID.insert(std::make_pair(triggerTime, xid));
        _idCallbackParam.insert(std::make_pair(xid, std::make_pair(cb, param)));
        return xid;
    }

    void XXTime::setPauseFlag(bool flag)
    {
        _bPause = flag;
    }

    bool XXTime::getPauseFlag()
    {
        return _bPause;
    }

    xid_t XXTime::killTimer(xid_t timerId)
    {
        auto idIter = _idCallbackParam.find(timerId);
        if (idIter != _idCallbackParam.end())
        {
            idIter->second.second._killed = true;
        }

        return timerId;
    }

    xid_t XXTime::generateTimerId()
    {
        xid_t  xid = 0;

        do
        {
            ++_timerIdMax;
            if (_timerIdMax > static_cast<xid_t>(-4))
            {
                _timerIdMax = 0;
            }

            if (_idCallbackParam.find(_timerIdMax) == _idCallbackParam.end())
            {
                xid = _timerIdMax;
                break;
            }

            //LOG_ERROR << "Timer id is already used: timerId=" << _timerIdMax << endl;
        }
        while(true);
        return xid;
    }

    int XXTime::getPeriodTimes()
    {
        return _periodTimes;
    }

    void XXTime::setPeriodTimes(int times)
    {
        _periodTimes = times;
    }
}
