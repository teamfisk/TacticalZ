#ifndef MiniDump_h__
#define MiniDump_h__

#include <DbgHelp.h>

#include <ctime>
#include <comdef.h>

long WINAPI Create_Dump_Immediate(LPEXCEPTION_POINTERS pException);

#endif
