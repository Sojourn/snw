#pragma once

#include <atomic>
#include <mutex>
#include <cassert>

namespace snw {

    using atomic_int64_t = std::atomic_int_fast64_t;

    class weak_mutex {
    public:
        weak_mutex()
            : version_(0)
        {}

        void lock() {
            assert((version_ & 1) == 0);
            version_++;
        }

        void unlock() {
            assert((version_ & 1) == 1);
            version_++;
        }

        int64_t version() const {
            return version_.load();
        }

    private:
        weak_mutex(weak_mutex&&) = delete;
        weak_mutex(const weak_mutex&) = delete;
        weak_mutex& operator=(weak_mutex&&) = delete;
        weak_mutex& operator=(const weak_mutex&) = delete;

        atomic_int64_t version_;
    };

    class weak_lock {
    public:
        weak_lock(weak_mutex& mutex)
            : mutex_(&mutex)
            , version_(mutex.version())
        {
        }

        bool is_locked() const {
            return ((version_ & 1) == 0);
        }

        bool is_quiescent() const {
            return is_locked() && (version_ == mutex_->version());
        }

        explicit operator bool() const {
            return is_quiescent();
        }

    private:
        weak_lock(weak_lock&&) = delete;
        weak_lock(const weak_lock&) = delete;
        weak_lock& operator=(weak_lock&&) = delete;
        weak_lock& operator=(const weak_lock&) = delete;

        weak_mutex* mutex_;
        int64_t     version_;
    };

}
