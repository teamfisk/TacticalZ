#if defined (_WIN64) | defined(_WIN32)
#ifndef WinLeakeCheck_h__
#define WinLeakeCheck_h__

//For memory leak checking
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif  // _DEBUG
#endif
#endif