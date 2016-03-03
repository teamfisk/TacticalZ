#include "Game.h"
#include "MiniDump.h"

LONG WINAPI CrashHandler(LPEXCEPTION_POINTERS pException);

int main(int argc, char* argv[])
{
    SetUnhandledExceptionFilter(CrashHandler);

    //run the .exe file to get a possible crashdump
    Game game(argc, argv);
    while (game.Running()) {
        game.Tick();
    }

    return 0;
}

LONG WINAPI CrashHandler(LPEXCEPTION_POINTERS pException)
{
    // Get absolute path of current process: C:/ ... /name.exe
    CHAR Dump_Path[MAX_PATH];
    GetModuleFileName(NULL, Dump_Path, sizeof(Dump_Path));
    std::string path(Dump_Path);
    // Then make the path into C:/ .. /nameDump.exe
    // Remove the .exe from path
    path = path.substr(0, path.length() - 4);
    path += "Dump.exe";

    wchar_t commandLine[32768];
    swprintf_s(commandLine, sizeof(commandLine) / sizeof(commandLine[0]), L"\"%ls\" %lu %lu %p", path.c_str(), GetCurrentProcessId(), GetCurrentThreadId(), pException);

    STARTUPINFOW sInfo;
    PROCESS_INFORMATION pInfo;
    ZeroMemory(&sInfo, sizeof(sInfo));
    ZeroMemory(&pInfo, sizeof(pInfo));
    sInfo.cb = sizeof(sInfo);

    if (CreateProcessW(L"TacticalZDump.exe",
        commandLine,
        nullptr,
        nullptr,
        FALSE,
        NORMAL_PRIORITY_CLASS,
        nullptr,
        nullptr,
        &sInfo,
        &pInfo)) {
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

