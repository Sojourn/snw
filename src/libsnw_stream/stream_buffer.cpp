#include "stream_buffer.h"
#include "align.h"
#include "platform.h"
#include <stdexcept>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cassert>

#if defined(SNW_OS_UNIX)

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>

namespace {
    static constexpr size_t page_size_ = 4096;

    // find a size that satisfies the following constraints:
    //    1. size > 0
    //    2. size >= min_size
    //    3. (size % page_size_) == 0
    //    4. is_power_of_2(size)
    //
    size_t find_size(size_t min_size) {
        if (min_size == std::numeric_limits<size_t>::max()) {
            // align_up would cause an overflow
            throw std::runtime_error("bad stream_buffer size");
        }

        size_t size = std::max(min_size, static_cast<size_t>(1));
        size = snw::align_up(size, page_size_);
        while (!snw::is_power_of_2(size)) {
            size += page_size_;
        }

        assert(size > 0);
        assert(size >= min_size);
        assert((size % page_size_) == 0);
        assert(snw::is_power_of_2(size));

        return size;
    }
}

snw::stream_buffer::stream_buffer(size_t min_size = 0)
    : size_(0)
    , data_(static_cast<uint8_t*>(MAP_FAILED))
    , fd_(-1)
{
    size_ = find_size(min_size);

    // TODO: Use scope guards or labels for error handling

    // create a temporary name for the shm
    char name[64];
    int pid = static_cast<int>(get_current_process_id());
    int tid = static_cast<int>(get_current_thread_id());
    if (snprintf(name, sizeof(name), "stream_buffer_%d_%d.shm", pid, tid) < 0) {
        throw std::runtime_error("failed create stream_buffer - snprintf");
    }

    // create the shm
    fd_ = shm_open(name, O_RDWR|O_CREAT|O_EXCL, 0644);
    if (fd_ < 0) {
        throw std::runtime_error("failed create stream_buffer - shm_open");
    }

    // unlink the shm (so that it doesn't stick around in /dev/shm in case we crash...)
    if (shm_unlink(name) < 0) {
        if (errno == ENOENT) {
            // removed, but not by us
        }
        else {
            int rc;
            rc = ::close(fd_);
            assert(rc >= 0);
            throw std::runtime_error("failed create stream_buffer - shm_unlink");
        }
    }

    // resize the shm
    if (ftruncate(fd_, size_) < 0) {
        int rc;
        rc = ::close(fd_);
        assert(rc >= 0);
        throw std::runtime_error("failed create stream_buffer - ftruncate");
    }

    // allocate enough virtual memory to fit the shm object twice (for the upper/lower mapping)
    void* lower_addr = mmap(nullptr, size_ * 2, PROT_READ|PROT_WRITE, MAP_SHARED, fd_, 0);
    if (lower_addr == MAP_FAILED) {
        int rc;
        rc = ::close(fd_);
        assert(rc >= 0);
        throw std::runtime_error("failed create stream_buffer - mmap 1");
    }

#if 0
    // this is needed if the os doesn't like us overwriting an existing mapping, but it
    // introduces a race
    void* new_lower_addr=  mremap(lower_addr, size_ * 2, size_, MREMAP_MAYMOVE, lower_addr);
    if (new_lower_addr == MAP_FAILED) {
        int rc;
        rc = ::close(fd_);
        assert(rc >= 0);
        rc = munmap(lower_addr, size_ * 2);
        assert(rc >= 0);
        throw std::runtime_error("failed create stream_buffer - mremap");
    }
    else if (new_lower_addr != lower_addr) {
        int rc;
        rc = ::close(fd_);
        assert(rc >= 0);
        rc = munmap(new_lower_addr, size_);
        assert(rc >= 0);
        throw std::runtime_error("failed create stream_buffer - mremap (moved)");
    }
#endif

    data_ = reinterpret_cast<uint8_t*>(lower_addr);

    // do the upper mapping
    void* upper_addr = mmap(data_+ size_, size_, PROT_READ|PROT_WRITE, MAP_FIXED|MAP_SHARED, fd_, 0);
    if (upper_addr == MAP_FAILED) {
        int rc;
        rc = ::close(fd_);
        assert(rc >= 0);
        rc = munmap(lower_addr, size_);
        assert(rc >= 0);
        throw std::runtime_error("failed create stream_buffer - mmap 2");
    }

    // fault pages and sanity check that we don't get a SIGSEGV
    memset(data_, 0, size_ * 2);
}

snw::stream_buffer::stream_buffer(stream_buffer&& other)
    : size_(other.size_)
    , data_(other.data_)
    , fd_(other.fd_)
{
    other.size_ = 0;
    other.data_ = static_cast<uint8_t*>(MAP_FAILED);
    other.fd_ = -1;
}

snw::stream_buffer::~stream_buffer() {
    close();
}

snw::stream_buffer& snw::stream_buffer::operator=(stream_buffer&& rhs) {
    if (this != &rhs) {
        close();

        size_ = rhs.size_;
        data_ = rhs.data_;
        fd_ = rhs.fd_;

        rhs.size_ = 0;
        rhs.data_ = static_cast<uint8_t*>(MAP_FAILED);
        rhs.fd_ = -1;
    }

    return *this;
}

snw::stream_buffer::operator bool() const {
    return (fd_ >= 0);
}

void snw::stream_buffer::close() {
    if (*this) {
        int rc;

        void* lower_addr = data_;
        rc = munmap(lower_addr, size_);
        assert(rc >= 0);

        void* upper_addr = data_ + size_;
        rc = munmap(upper_addr, size_);
        assert(rc >= 0);

        rc = ::close(fd_);
        assert(rc >= 0);

        size_ = 0;
        data_ = static_cast<uint8_t*>(MAP_FAILED);
        fd_ = -1;
    }
}

#else

snw::stream_buffer::stream_buffer(size_t min_size)
    : size_(0)
    , data_(NULL)
    , fd_(-1)
{
    throw std::runtime_error("not implemented");
}

snw::stream_buffer::stream_buffer(stream_buffer&& other)
    : size_(other.size_)
    , data_(other.data_)
    , fd_(other.fd_)
{
    throw std::runtime_error("not implemented");
}

snw::stream_buffer::~stream_buffer() {
    close();
}

snw::stream_buffer& snw::stream_buffer::operator=(stream_buffer&& rhs) {
    throw std::runtime_error("not implemented");
}

snw::stream_buffer::operator bool() const {
    return (fd_ >= 0);
}

void snw::stream_buffer::close() {
    throw std::runtime_error("not implemented");
}

#endif
