/*
	Author:	Vladimir Sedach.

	Purpose: demo of Call Stack creation by our own means,
	and with MiniDumpWriteDump() function of DbgHelp.dll.
*/

#include <iostream>
#include <ctime>

#include <windows.h>
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

typedef enum _MINIDUMP_TYPE {
	MiniDumpNormal =			0x00000000,
	MiniDumpWithDataSegs =		0x00000001,
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
void WINAPI Create_Dump(PEXCEPTION_POINTERS pException, BOOL File_Flag, BOOL Show_Flag)
//*************************************************************************************
// Create dump. 
// pException can be either GetExceptionInformation() or NULL.
// If File_Flag = TRUE - write dump files (.dmz and .dmp) with the name of the current process.
// If Show_Flag = TRUE - show message with Get_Exception_Info() dump.
{
    // Try to get MiniDumpWriteDump() address.
    hDbgHelp = LoadLibrary("DBGHELP.DLL");
    MiniDumpWriteDump_ = (MINIDUMP_WRITE_DUMP)GetProcAddress(hDbgHelp, "MiniDumpWriteDump");

	// If MiniDumpWriteDump() of DbgHelp.dll available.
	if (MiniDumpWriteDump_)
	{
	    HANDLE	hDump_File;
	    CHAR	Dump_Path[MAX_PATH];

	    GetModuleFileName(NULL, Dump_Path, sizeof(Dump_Path));	//path of current process
        std::time_t t = std::time(NULL);
        char tStr[16];
        std::strftime(tStr, 32, " %a %H-%M-%S", std::localtime(&t));
        std::string time(tStr);
        std::string path(Dump_Path);
        path = path.substr(0, path.length() - 4);
        path += time + ".dmp";

		MINIDUMP_EXCEPTION_INFORMATION	M;
		M.ThreadId = GetCurrentThreadId();
		M.ExceptionPointers = pException;
		M.ClientPointers = 0;

		hDump_File = CreateFile(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		MiniDumpWriteDump_(GetCurrentProcess(), GetCurrentProcessId(), hDump_File,
			MiniDumpNormal, (pException) ? &M : NULL, NULL, NULL);

		CloseHandle(hDump_File);

        std::cout << "Memory dumped to: \"" << path.c_str() << "\"";
        MessageBox(NULL, ("Application crashed, memory dumped to: " + path).c_str(), "MiniDump", MB_ICONHAND | MB_OK);
    } else {
        MessageBox(NULL, "Application crashed, memory dump failed.", "MiniDump", MB_ICONHAND | MB_OK);
    }
}

