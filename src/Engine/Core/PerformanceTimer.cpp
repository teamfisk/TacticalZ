#include "Core/PerformanceTimer.h"

#ifdef DEBUG
#include <ctime>
#include <fstream>


cpu_timer PerformanceTimer::m_Timer;
std::map<std::string, cpu_timer> PerformanceTimer::timers;
std::string PerformanceTimer::currentTimerRunning = "";

void PerformanceTimer::StartTimer(std::string nameOfTimer)
{
    timers[nameOfTimer].stop();
    timers[nameOfTimer].start();
    currentTimerRunning = nameOfTimer;
}

void PerformanceTimer::StartTimerAndStopPrevious(std::string nameOfTimer)
{
    //stop the current timer and start some other - useful to not have to stop timers all the time
    if (currentTimerRunning != "") {
        timers[currentTimerRunning].stop();
    }
    timers[nameOfTimer].stop();
    timers[nameOfTimer].start();
    currentTimerRunning = nameOfTimer;
}

void PerformanceTimer::StopTimer(std::string nameOfTimer)
{
    timers[nameOfTimer].stop();
    currentTimerRunning = nameOfTimer;
}

void PerformanceTimer::SetFrameNumber(int frameNumber)
{
}

void PerformanceTimer::ResetAllTimers()
{
    //stop all timers
    for (auto aTimer : timers)
    {
        aTimer.second.stop();
    }
    currentTimerRunning = "";
    timers.clear();
}

void PerformanceTimer::CreateExcelData()
{
    //get time
    std::time_t t = std::time(NULL);
    char tStr[16];
    std::strftime(tStr, 32, " %a %H-%M-%S", std::localtime(&t));
    std::string time(tStr);
    std::string path("TacticalZ");
    path += time + ".csv";
    std::ofstream someFileStream;
    someFileStream.open(path, std::ofstream::out);
    someFileStream << "classname" << ',' << "walltime" << ',' << "userTime" << ',' << "systemTime" << '\n';

    //write all timers to file
    for (auto aTimer : timers)
    {
        //remove the "class" name in front of the string
        auto className = aTimer.first;
        if (className.find("class ") != std::string::npos) {
            className.replace(0, 6, "");
        }
        auto wallTime = (double)aTimer.second.elapsed().wall*1e-3;
        auto userTime = (double)aTimer.second.elapsed().user*1e-3;
        auto systemTime = (double)aTimer.second.elapsed().system*1e-3;

        someFileStream << className << "," << wallTime << ',' << userTime << ',' << systemTime << '\n';
    }
    someFileStream.close();
}

#else

void PerformanceTimer::StartTimer(std::string nameOfTimer) {};
void PerformanceTimer::StartTimerAndStopPrevious(std::string nameOfTimer) {};
void PerformanceTimer::StopTimer(std::string nameOfTimer) {};
void PerformanceTimer::SetFrameNumber(int frameNumber) {};

void PerformanceTimer::ResetAllTimers() {};
void PerformanceTimer::CreateExcelData() {};

#endif //DEBUG
