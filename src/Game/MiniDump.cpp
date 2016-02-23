/*
	Author:	Vladimir Sedach.

	Purpose: demo of Call Stack creation by our own means,
	and with MiniDumpWriteDump() function of DbgHelp.dll.
*/

#include <iostream>
#include <ctime>

#include <windows.h>
#include <comdef.h>
#include <tlhelp32.h>
//#include "dbghelp.h"

//#define DEBUG_DPRINTF		1	//allow d()
//#include "wfun.h"

#pragma optimize("y", off)		//generate stack frame pointers for all functions - same as /Oy- in the project
#pragma warning(disable: 4200)	//nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable: 4100)	//unreferenced formal parameter

// In case you don't have dbghelp.h.
#ifndef _DBGHELP_

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
	DWORD	ThreadId;
	PEXCEPTION_POINTERS	ExceptionPointers;
	BOOL	ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef enum _MINIDUMP_TYPE
{
    MiniDumpNormal = 0x00000000,
    MiniDumpWithDataSegs = 0x00000001,
    MiniDumpWithFullMemory = 0x00000002,
    MiniDumpWithHandleData = 0x00000004,
    MiniDumpFilterMemory = 0x00000008,
    MiniDumpScanMemory = 0x00000010,
    MiniDumpWithUnloadedModules = 0x00000020,
    MiniDumpWithIndirectlyReferencedMemory = 0x00000040,
    MiniDumpFilterModulePaths = 0x00000080,
    MiniDumpWithProcessThreadData = 0x00000100,
    MiniDumpWithPrivateReadWriteMemory = 0x00000200,
    MiniDumpWithoutOptionalData = 0x00000400,
    MiniDumpWithFullMemoryInfo = 0x00000800,
    MiniDumpWithThreadInfo = 0x00001000,
    MiniDumpWithCodeSegs = 0x00002000,
    MiniDumpWithoutManagedState = 0x00004000,
} MINIDUMP_TYPE;

typedef	BOOL (WINAPI * MINIDUMP_WRITE_DUMP)(
	IN HANDLE			hProcess,
	IN DWORD			ProcessId,
	IN HANDLE			hFile,
	IN MINIDUMP_TYPE	DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam, OPTIONAL
	IN PVOID									UserStreamParam, OPTIONAL
	IN PVOID									CallbackParam OPTIONAL
	);

#else

typedef	BOOL (WINAPI * MINIDUMP_WRITE_DUMP)(
	IN HANDLE			hProcess,
	IN DWORD			ProcessId,
	IN HANDLE			hFile,
	IN MINIDUMP_TYPE	DumpType,
	IN CONST PMINIDUMP_EXCEPTION_INFORMATION	ExceptionParam, OPTIONAL
	IN PMINIDUMP_USER_STREAM_INFORMATION		UserStreamParam, OPTIONAL
	IN PMINIDUMP_CALLBACK_INFORMATION			CallbackParam OPTIONAL
	);
#endif //#ifndef _DBGHELP_

HMODULE	hDbgHelp;
MINIDUMP_WRITE_DUMP	MiniDumpWriteDump_;

// Tool Help functions.
typedef	HANDLE (WINAPI * CREATE_TOOL_HELP32_SNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);

//*************************************************************************************
void WINAPI Create_Dump(PEXCEPTION_POINTERS pException)
//*************************************************************************************
// Create dump. 
// pException can be either GetExceptionInformation() or NULL.
{
    // Try to get MiniDumpWriteDump() address.
    hDbgHelp = LoadLibrary("DBGHELP.DLL");
    MiniDumpWriteDump_ = (MINIDUMP_WRITE_DUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

    // If MiniDumpWriteDump() of DbgHelp.dll available.
    if (MiniDumpWriteDump_)
    {
        //get absolute path of current process: C:/ ... /name.exe
        CHAR Dump_Path[MAX_PATH];
        GetModuleFileName(NULL, Dump_Path, sizeof(Dump_Path));
        std::string path(Dump_Path);

        //Get current time in a string.
        std::time_t t = std::time(NULL);
        char tStr[16];
        std::strftime(tStr, ARRAYSIZE(tStr), " %a %H-%M-%S", std::localtime(&t));
        std::string time(tStr);
        // Remove the .exe from path
        path = path.substr(0, path.length() - 4);
        // Add the current time and .dmp
        path += time + ".dmp";

        MINIDUMP_EXCEPTION_INFORMATION M;
        M.ThreadId = GetCurrentThreadId();
        M.ExceptionPointers = pException;
        M.ClientPointers = TRUE;

        HANDLE hDump_File = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        
        DWORD curProId = GetCurrentProcessId();
        //HANDLE curPro = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE, FALSE, curProId);
        HANDLE curPro = GetCurrentProcess();

        BOOL result = MiniDumpWriteDump_(curPro, curProId, hDump_File,
            MiniDumpNormal, (pException) ? &M : NULL, NULL, NULL);
        HRESULT error = (HRESULT)GetLastError();

        CloseHandle(hDump_File);
        if (!result) {
            _com_error cErr(error);
            HRESULT actualErrorCode = error & 0xFFFF;
            char eBuf[16];
            std::sprintf(eBuf, "0x%x", actualErrorCode);
            std::cout << "Bad memory dump at: \"" << path.c_str() << "\"" << std::endl
                << "because MiniDumpWriteDump failed with error #" << actualErrorCode << " (" << eBuf << "): \"" << cErr.ErrorMessage() << "\"" << std::endl;
            MessageBox(NULL, "Application crashed, memory dump failed, but file was created.", "MiniDump", MB_ICONHAND | MB_OK);
        } else {
            std::cout << "Memory dumped to: \"" << path.c_str() << "\"";
            MessageBox(NULL, ("Application crashed, memory dumped to: " + path).c_str(), "MiniDump", MB_ICONHAND | MB_OK);
        }
    } else {
        std::cout << "Memory dump failed because MiniDumpWriteDump is not available.";
        MessageBox(NULL, "Application crashed, could not create a memory dump.", "MiniDump", MB_ICONHAND | MB_OK);
    }
}

