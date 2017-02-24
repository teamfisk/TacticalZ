#include "Game.h"
#include "MiniDump.h"

int main(int argc, char* argv[])
{
    SetUnhandledExceptionFilter(Create_Dump_Immediate);

    Game game(argc, argv);
    while (game.Running()) {
        PerformanceTimer::StartTimer("Game-Tick");
        game.Tick();
        PerformanceTimer::StopTimer("Game-Tick");
    }

    return 0;
}
