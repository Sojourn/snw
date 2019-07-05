#pragma once

#if defined(_WIN32) || defined(_WIN64)
#  define SNW_OS_WINDOWS (1)
#elif defined(unix) || defined(__unix) || defined(__unix__)
#  define SNW_OS_UNIX (1)
#  if defined(__linux__)
#    define SNW_OS_LINUX (1)
#  endif
#endif

namespace snw {

using process_id = int;
using thread_id = int;

process_id get_current_process_id();
thread_id get_current_thread_id();

}
