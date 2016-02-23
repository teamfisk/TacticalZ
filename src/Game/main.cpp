//#include "Game.h"
#include "MiniDump.h"

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* pException);

int main(int argc, char* argv[])
{
    //::SetUnhandledExceptionFilter(CrashHandler);

    //EXCEPTION_POINTERS* pException = new EXCEPTION_POINTERS();
    //pException->ContextRecord = new _CONTEXT();
    //pException->ExceptionRecord = new EXCEPTION_RECORD();
    //CrashHandler(pException);
    //return 0;

    __try {
        int* ii = nullptr;
        *ii = 17;

        //int x = asd[4];
        //std::cout << "crash before we get here, don't optimize x. " << x;

        //throw std::exception();
    } __except (CrashHandler(GetExceptionInformation())) {
        system("pause");
        return 0;
    }
    system("pause");
    /*
    Game game(argc, argv);
    while (game.Running()) {
        game.Tick();
    }
    */
    return 0;
}

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* pException)
{
    //Take minidump. path should be bin/TacticalZ.dmp
    //Then show MessageBox, and exit application.
    Create_Dump(pException);

    return EXCEPTION_EXECUTE_HANDLER;// EXCEPTION_CONTINUE_SEARCH
}