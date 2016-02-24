#include "Game.h"
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
    // Get absolute path of current process: C:/ ... /name.exe
    CHAR Dump_Path[MAX_PATH];
    GetModuleFileName(NULL, Dump_Path, sizeof(Dump_Path));
    std::string path(Dump_Path);
    // Then make the path into C:/ .. /nameDump.exe
    // Remove the .exe from path
    path = path.substr(0, path.length() - 4);
    path += "Dump.exe";

    DWORD procId = GetCurrentProcessId();
    DWORD threadId = GetCurrentThreadId();
    std::uintptr_t intExcPtr = reinterpret_cast<std::uintptr_t>(pException);
    std::ostringstream ss;
    ss << procId << " " << threadId << " " << intExcPtr;
    char cmds[128];
    strcpy(cmds, ss.str().c_str());

    STARTUPINFO sInfo;
    PROCESS_INFORMATION pInfo;
    ZeroMemory(&sInfo, sizeof(sInfo));
    ZeroMemory(&pInfo, sizeof(pInfo));
    sInfo.cb = sizeof(sInfo);

    if (CreateProcess(path.c_str(),
        cmds,
        nullptr,
        nullptr,
        FALSE,
        NORMAL_PRIORITY_CLASS,
        nullptr,
        nullptr,
        &sInfo,
        &pInfo)
        ) {
        WaitForSingleObject(pInfo.hProcess, INFINITE);
        CloseHandle(pInfo.hProcess);
        CloseHandle(pInfo.hThread);
    } else {
        HRESULT error = ((HRESULT)GetLastError()) & 0xFFFF;
        char hexErrBuf[16];
        std::sprintf(hexErrBuf, "0x%x", error);
        std::cout << "Could not create a minidump because CreateProcess failed with error " << hexErrBuf;
        MessageBox(NULL, "Application crashed, could not create a memory dump.", "MiniDump", MB_ICONHAND | MB_OK);
    }

    return EXCEPTION_EXECUTE_HANDLER;// EXCEPTION_CONTINUE_SEARCH
}