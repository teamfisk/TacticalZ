/*
Inspired by Original Author: Vladimir Sedach.

Purpose: demo of Call Stack creation by our own means,
and with MiniDumpWriteDump() function of DbgHelp.dll.
*/

#include "MiniDump.h"

#include <string>
#include <iostream>
#include <ctime>
#include <comdef.h>

#pragma optimize("y", off)      //generate stack frame pointers for all functions - same as /Oy- in the project
#pragma warning(disable: 4200)  //nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4100)  //unreferenced formal parameter

typedef	BOOL(WINAPI * MINIDUMP_WRITE_DUMP)(
    IN HANDLE           hProcess,
    IN DWORD            ProcessId,
    IN HANDLE           hFile,
    IN MINIDUMP_TYPE    DumpType,
    IN CONST PMINIDUMP_EXCEPTION_INFORMATION    ExceptionParam, OPTIONAL
    IN PVOID                                    UserStreamParam, OPTIONAL
    IN PVOID                                    CallbackParam OPTIONAL
    );

//*************************************************************************************
long WINAPI Create_Dump_Immediate(LPEXCEPTION_POINTERS pException)
//*************************************************************************************
// Create dump. 
// pException can be either GetExceptionInformation() or NULL.
{
    // Try to get MiniDumpWriteDump() address.
    HMODULE hDbgHelp = LoadLibrary("DBGHELP.DLL");
    MINIDUMP_WRITE_DUMP MiniDumpWriteDump_ = (MINIDUMP_WRITE_DUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

    // If MiniDumpWriteDump() of DbgHelp.dll available.
    if (MiniDumpWriteDump_) {
        // Get absolute path of current process: C:/ ... /name.exe
        CHAR Dump_Path[MAX_PATH];
        GetModuleFileName(NULL, Dump_Path, sizeof(Dump_Path));
        std::string path(Dump_Path);

        // Get current time in a string.
        std::time_t t = std::time(NULL);
        char tStr[16];
        std::strftime(tStr, ARRAYSIZE(tStr), " %a %H-%M-%S", std::localtime(&t));
        std::string time(tStr);
        // Remove the .exe from path
        path = path.substr(0, path.length() - 4);
        // Add the current time and .dmp
        path += "dump" + time + ".dmp";

        DWORD procId = GetCurrentProcessId();
        DWORD threadId = GetCurrentThreadId();

        MINIDUMP_EXCEPTION_INFORMATION M;
        M.ThreadId = threadId;
        M.ExceptionPointers = pException;
        M.ClientPointers = TRUE;

        HANDLE hDump_File = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE process = GetCurrentProcess();

        BOOL result = MiniDumpWriteDump_(process, procId, hDump_File
            , (MINIDUMP_TYPE)(MiniDumpWithDataSegs |
                MiniDumpWithHandleData |
                MiniDumpScanMemory |
                MiniDumpWithUnloadedModules |
                MiniDumpWithIndirectlyReferencedMemory |
                MiniDumpWithPrivateReadWriteMemory |
                MiniDumpWithFullMemoryInfo |
                MiniDumpWithThreadInfo |
                MiniDumpIgnoreInaccessibleMemory)
            , &M, NULL, NULL);
        HRESULT error = (HRESULT)GetLastError();

        CloseHandle(hDump_File);
        if (!result) {
            _com_error cErr(error);
            HRESULT actualErrorCode = error & 0xFFFF;
            char hexErrBuf[16];
            std::sprintf(hexErrBuf, "0x%x", actualErrorCode);
            std::cout << "Bad memory dump at: \"" << path.c_str() << "\"" << std::endl
                << "because MiniDumpWriteDump failed with error #" << actualErrorCode << " (" << hexErrBuf << "): \"" << cErr.ErrorMessage() << "\"" << std::endl;
            MessageBox(NULL, "Application crashed, memory dump failed, but file was created.", "MiniDump", MB_ICONHAND | MB_OK);
        } else {
            std::cout << "Memory dumped to: \"" << path.c_str() << "\"" << std::endl;
            MessageBox(NULL, ("Application crashed, memory dumped to: " + path).c_str(), "MiniDump", MB_ICONHAND | MB_OK);
        }
    } else {
        std::cout << "Memory dump failed because MiniDumpWriteDump is not available." << std::endl;
        MessageBox(NULL, "Application crashed, could not create a memory dump.", "MiniDump", MB_ICONHAND | MB_OK);
    }

    // Basically, terminate the application.
    return EXCEPTION_EXECUTE_HANDLER;
}

