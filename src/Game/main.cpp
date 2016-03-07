#include "Game.h"
#include "MiniDump.h"

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* pException);

int main(int argc, char* argv[])
{
    ::SetUnhandledExceptionFilter(CrashHandler);

    Game game(argc, argv);
    while (game.Running()) {
		PerformanceTimer::StartTimer("Game-Tick");
        game.Tick();
		PerformanceTimer::StopTimer("Game-Tick");
    }

	return 0;
}

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* pException)
{
    //Take minidump. path should be bin/TacticalZ.dmp
    //Then show MessageBox, and exit application.
    Create_Dump(pException, 1, 1);

    return EXCEPTION_EXECUTE_HANDLER;// EXCEPTION_CONTINUE_SEARCH
}