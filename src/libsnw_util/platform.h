#pragma once

#if defined(_WIN32) || defined(_WIN64)
#  define SNW_OS_WINDOWS (1)
#elif defined(unix) || defined(__unix) || defined(__unix__)
#  define SNW_OS_UNIX (1)
#  if defined(__linux__)
#    define SNW_OS_LINUX (1)
#  endif
#endif
