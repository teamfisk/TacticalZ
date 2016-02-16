#ifndef PerformanceTimer_h__
#define PerformanceTimer_h__

#include <array>
#include <GLFW/glfw3.h>
#include "../Common.h"

#include <boost/timer/timer.hpp>
using boost::timer::cpu_timer;

class PerformanceTimer
{
public:
    static void StartTimer(std::string nameOfTimer);
    static void StartTimerAndStopPrevious(std::string nameOfTimer);
    static void StopTimer(std::string nameOfTimer);
    static void SetFrameNumber(int frameNumber);

    static void ResetAllTimers();
    static void CreateExcelData();

    //set timer/start
    //get performance excel nånting

private:
    static std::map<std::string, cpu_timer> timers;
    static double m_TimeElapsed;
    static cpu_timer m_Timer;
    static std::string currentTimerRunning;
    static bool active;
    //static map


};

#endif
