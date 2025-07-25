#ifndef __GAME_XTIMER_H__
#define __GAME_XTIMER_H__

#include <stdint.h>
#include <memory>
#include <map>
#include <vector>
#include <functional>
#include "singleton.h"

namespace XTimer
{
    //最多可添加68719476736个定时器
    typedef uint64_t xid_t;
    //以秒为单位最多可连续运行2209年
    typedef uint64_t xtime_t;
    //用户结构体的资源释放
    typedef void *(*DestructorFunc_t)(void **);

    class XXTime;
    class TimerParam
    {
        friend class XXTime;

    public:
        TimerParam()
            : _interval(0)
            , _timerKey(-1)
            , _userStruct(NULL)
            , _destructor(NULL)
            , _killed(false)
            , _triggerTime(0)
        {}

        TimerParam(xtime_t interval)
            : _interval(interval)
            , _timerKey(-1)
            , _userStruct(NULL)
            , _destructor(NULL)
            , _killed(false)
            , _triggerTime(0)
        {}

        TimerParam(xtime_t interval, int timer_key)
            : _interval(interval)
            , _timerKey(timer_key)
            , _userStruct(NULL)
            , _destructor(NULL)
            , _killed(false)
            , _triggerTime(0)
        {}

        TimerParam(xtime_t interval, int timer_key, void *user_struct, DestructorFunc_t destructor)
            : _interval(interval)
            , _timerKey(timer_key)
            , _userStruct(user_struct)
            , _destructor(destructor)
            , _killed(false)
            , _triggerTime(0)
        {}

    public:
        int getKey() const
        {
            return _timerKey;
        }

        void const *getBody() const
        {
            return _userStruct;
        }

        xtime_t getInterval() const
        {
            return _interval;
        }

    protected:
        //
        xtime_t _interval;
        //
        int _timerKey;
        //
        void *_userStruct;
        //
        DestructorFunc_t _destructor;
        //
        bool _killed;
        //
        xtime_t _triggerTime;
    };

    /**
     *
    */
    class XXTime : public Singleton<XXTime>
    {
        //
        friend int onTickTock(int taskNum);
        //
        friend inline void setPauseFlag(bool flag);
        //
        friend inline bool getPauseFlag();
        //
        friend xid_t setTimer(xtime_t lifeTime, std::function<int(TimerParam &)> cb, TimerParam  param);
        //
        friend xid_t killTimer(xid_t timerId);
        //
        friend xtime_t getTimerRemaining(xid_t timerId);

    public:
        XXTime(): _timerIdMax(0), _currentTime(0), _bPause(false), _periodTimes(10)
        {
            _triggerTimeID.clear();
            _idCallbackParam.clear();
        }

    protected:
        ///
        // Function:时钟走秒
        // Describe:
        // Return:正常返回0,异常-1
        // Param
        ///
        int onTickTock(int taskNum);

        ///
        // Function:设定有主定时器
        // Describe:
        // Return: 成功设定的timerId
        ///
        xid_t setTimer(xtime_t lifeTime, std::function<int(TimerParam &)> cb, TimerParam param);

        ///
        // Function:设定暂停标记
        // Describe:
        // Return:
        ///
        void setPauseFlag(bool flag);

        ///
        // Function:取暂停标记
        // Describe:
        // Return:
        ///
        bool getPauseFlag();

        ///
        // Function:获取定时器剩余时间
        // Describe:
        // Return:剩余到期时间
        ///
        xtime_t getTimerRemaining(xid_t timerId);

        ///
        // Function:关闭闹钟
        // Describe:
        // Return:成功返回 0,异常static_cast<xid_t>(-1)
        // Param timerId 关闭的Id
        ///
        xid_t killTimer(xid_t timerId);

        ///
        // Function:算出一个可用的timerId;
        // Describe:用到上限后从开始处循环使用;
        // Return: timerId;
        // Param
        ///
        xid_t generateTimerId();

    public:
        ///
        // Function:获取定时器每秒中调用频繁
        // Describe:用到上限后从开始处循环使用;
        // Return: timerId;
        // Param
        ///
        int getPeriodTimes();

        ///
        // Function:设置定时器每秒中调用频繁
        // Describe:用到上限后从开始处循环使用;
        // Return: timerId;
        // Param
        ///
        void setPeriodTimes(int times);

    protected:
        //
        xid_t _timerIdMax;
        //
        xtime_t _currentTime;
        //
        std::map<xid_t, std::pair<std::function<int(TimerParam &)>, TimerParam>> _idCallbackParam;
        //
        std::multimap<xtime_t, xid_t> _triggerTimeID;
        //
        bool _bPause;
        //
        int _periodTimes;
    };

    typedef XXTime XTime;
    //
    inline int onTickTock(int taskNum)
    {
        return XTime::getInstance()->onTickTock(taskNum);
    }
    //
    inline xid_t setTimer(xtime_t lifeTime, std::function<int(TimerParam &)> cb, TimerParam  param)
    {
        return XTime::getInstance()->setTimer(lifeTime, cb, param);
    }
    //
    inline void setPauseFlag(bool flag)
    {
        XTime::getInstance()->setPauseFlag(flag);
    }
    //
    inline bool getPauseFlag()
    {
        return XTime::getInstance()->getPauseFlag();
    }
    //
    inline xtime_t getTimerRemaining(xid_t timerId)
    {
        return XTime::getInstance()->getTimerRemaining(timerId);
    }
    //
    inline xid_t killTimer(xid_t timerId)
    {
        return XTime::getInstance()->killTimer(timerId);
    }
    //
    inline void setPeriodTimes(int times)
    {
        return XTime::getInstance()->setPeriodTimes(times);
    }

    template<typename StructT>
    void *Destructor(void **p)
    {
        if (p && *p)
        {
            delete static_cast<StructT *>(*p);
            *p = NULL;
        }

        return *p;
    }
    //
    template<typename T, typename... Args>
    T *make_body(Args &&... args)
    {
        return new T(std::forward<Args>(args)...);
    }
}

#endif /*__GAME_XTIMER_H__*/
