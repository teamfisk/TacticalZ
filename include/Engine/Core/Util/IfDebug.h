//Example:
//IF_DEBUG_IS(true) {
//    //Stuff done only in debug mode.
//}
//
//IF_DEBUG_IS(false) {
//    //Stuff done only in release mode.
//} else {
//    //Stuff only in debug.
//}
#ifndef IF_DEBUG_IS
#ifndef DEBUG
#define IF_DEBUG_IS(c) if(c)
#else
#define IF_DEBUG_IS(c) if(!c)
#endif
#endif