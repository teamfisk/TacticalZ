#ifndef MiniDump_h__
#define MiniDump_h__

#include <Windows.h>

void WINAPI Create_Dump(PEXCEPTION_POINTERS pException, BOOL File_Flag, BOOL Show_Flag);

#endif
