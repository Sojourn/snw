#include "platform.h"

#if defined(SNW_OS_UNIX)
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#elif defined(SNW_OS_WINDOWS)
#include <Windows.h>
#endif

snw::process_id snw::get_current_process_id() {
#if defined(SNW_OS_UNIX)
    return ::getpid();
#elif defined(SNW_OS_WINDOWS)
    return GetCurrentProcessId();
#else
#error "not implemented"
#endif
}

snw::thread_id snw::get_current_thread_id() {
#if defined(SNW_OS_UNIX)
    return syscall(SYS_gettid);
#elif defined(SNW_OS_WINDOWS)
    return GetCurrentThreadId();
#else
#error "not implemented"
#endif
}
