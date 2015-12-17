// Example:
// DEBUG_IF(condition) {
//    // This code is executed only in debug mode and if condition is true.
// }
// NOTE: condition statement is not executed at all in release mode.
#ifndef DEBUG_IF
#ifndef DEBUG
#define DEBUG_IF(c) if(c)
#else
#define DEBUG_IF(c) if(false)
#endif
#endif