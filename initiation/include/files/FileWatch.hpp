#ifdef __linux__
#include "linux/FileWatchLinux.hpp"
#elif __MINGW32__
#error "Not supported"
#else
#error "Not supported"
#endif