#pragma once

#include <vector>
#include <algorithm>
#include <type_traits>
#include <cstdint>

namespace snw {

template<typename T, typename Key=int64_t, Key initial_key=Key{}>
class registry {
public:
    registry()
        : size_(0)
        , next_key_(initial_key)
    {
    }

    bool empty() const {
        return size_ = 0;
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return slots_.capacity();
    }

    template<typename... Args>
    const Key& emplace(const Key& key, Args&&... args) {
        slots_.emplace(next_key_++, std::forward<Args>(args));
        ++size_;
        return slots_.back().get_key();
    }

    void erase(const Key& key) {
        auto it = std::lower_bound(slots_.begin(), slots_.end(), key);
        if (it != slots_.end() && *it && it->get_key() == key) {
            it->clear();
            --size_;
        }
    }

    void clear() {
        slots_.clear();
        size_ = 0;
    }

    void vacuum() {
        auto beg = slots_.begin();
        auto end = slots_.end();
        auto pos = std::remove_if(beg, end, [&](const slot& s) {
            return !static_cast<bool>(s);
        });

        if (pos != end) {
            size_ -= std::distance(pos, end);
            slots_.erase(pos, end);
        }
    }

    T* find(const Key& key) {
        auto it = std::lower_bound(slots_.begin(), slots_.end(), key);
        if (it != slots_.end() && *it && it->get_key() == key) {
            return &it->get_value();
        }

        return nullptr;
    }

    const T* find(const Key& key) const {
        auto it = std::lower_bound(slots_.begin(), slots_.end(), key);
        if (it != slots_.end() && *it && it->get_key() == key) {
            return &it->get_value();
        }

        return nullptr;
    }

    template<typename F>
    void for_each(F&& f) {
        for (auto&& s: slots_) {
            if (s) {
                f(s.get_key(), s.get_value());
            }
        }
    }

    template<typename F>
    void for_each(F&& f) const {
        for (auto&& s: slots_) {
            if (s) {
                f(s.get_key(), s.get_value());
            }
        }
    }

private:
    class slot {
    public:
        slot()
            : active_(false)
        {
        }

        template<typename... Args>
        slot(const Key& key, Args&&... args)
            : active_(true)
            , key_(key)
        {
            new(&storage_) T(std::forward<Args>(args)...);
        }

        slot(slot&& other)
            : active_(other.active_)
            , key_(other.key_)
        {
            if (other) {
                new(&storage_) T(std::move(other.get_value()));
                other.clear();
            }
        }

        slot(const slot&) = delete;

        ~slot() {
            clear();
        }

        slot& operator=(slot&& rhs) {
            if (this != &rhs) {
                if (*this) {
                    clear();
                }

                if (rhs) {
                    active_ = true;
                    key_ = rhs.key_;
                    new(&storage_) T(std::move(rhs.get_value()));
                    rhs.clear();
                }
            }

            return *this;
        }

        slot& operator=(const slot&) = delete;

        explicit operator bool() const {
            return active_;
        }

        bool empty() const {
            return !active_;
        }

        const Key& get_key() const {
            assert(active_);
            return key_;
        }

        T& get_value() {
            assert(active_);
            return *static_cast<T*>(&storage_);
        }

        const T& get_value() const {
            assert(active_);
            return *static_cast<const T*>(&storage_);
        }

        void clear() {
            if (*this) {
                get_value().~T();
                active_ = false;
            }
        }

        friend bool operator==(const slot& lhs, const slot& rhs) {
            return lhs.get_key() == rhs.get_key();
        }

        friend bool operator==(const slot& lhs, const Key& rhs) {
            return lhs.get_key() == rhs;
        }

        friend bool operator!=(const slot& lhs, const slot& rhs) {
            return lhs.get_key() != rhs.get_key();
        }

        friend bool operator!=(const slot& lhs, const Key& rhs) {
            return lhs.get_key() != rhs;
        }

        friend bool operator<(const slot& lhs, const slot& rhs) {
            return lhs.get_key() < rhs.get_key();
        }

        friend bool operator<(const slot& lhs, const Key& rhs) {
            return lhs.get_key() < rhs;
        }

    private:
        bool                                          active_;
        Key                                           key_;
        std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
    };

    std::vector<slot> slots_;
    size_t            size_;
    Key               next_key_;
};

}
