#ifndef MiniDump_h__
#define MiniDump_h__

#include <Windows.h>
#include <DbgHelp.h>

long WINAPI Create_Dump_Immediate(LPEXCEPTION_POINTERS pException);

#endif
