#ifndef PerformanceTimer_h__
#define PerformanceTimer_h__

#include "../Common.h"

#ifdef DEBUG
#include <boost/timer/timer.hpp>
using boost::timer::cpu_timer;
#endif //DEBUG

class PerformanceTimer
{
public:
    static void StartTimer(std::string nameOfTimer);
    static void StartTimerAndStopPrevious(std::string nameOfTimer);
    static void StopTimer(std::string nameOfTimer);
    static void SetFrameNumber(int frameNumber);

    static void ResetAllTimers();
    static void CreateExcelData();

private:
#ifdef DEBUG
    static std::map<std::string, cpu_timer> timers;
    static cpu_timer m_Timer;
    static std::string currentTimerRunning;
#endif //DEBUG
};

#endif
