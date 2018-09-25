#pragma once

#include "box.h"

namespace snw {

    template<typename T>
    class future;

    template<typename T>
    class promise;

    template<typename T>
    class future {
        template<typename T_>
        friend class promise;

        template<typename T_>
        friend struct future_promise;

        template<typename T_>
        friend future<T_> make_ready_future(T_);
    public:
        future();
        future(future&& other);
        future(const future&) = delete;
        ~future();

        future& operator=(future&& rhs);
        future& operator=(const future&) = delete;

        bool is_broken() const;
        bool is_waiting() const;
        // bool is_ready() const;
        bool has_value() const;
        T& value();
        const T& value() const;

    private:
        future(promise<T>* promise);
        future(T value);

        enum class state {
            broken,
            waiting,
            ready,
        };

        promise<T>* promise_;
        state       state_;
        box<T>      value_;
    };

    template<typename T>
    class promise {
        template<typename T_>
        friend class future;

        template<typename T_>
        friend struct future_promise;
    public:
        promise();
        promise(promise&& other);
        promise(const promise&) = delete;
        ~promise();

        promise& operator=(promise&& rhs);
        promise& operator=(const promise&) = delete;

        void set_value(T value);

    private:
        promise(future<T>* future);

        future<T>* future_;
    };

    template<typename T>
    struct future_promise {
        snw::future<T> future;
        snw::promise<T> promise;

        future_promise(): future(&promise), promise(&future) {}
    };

    template<typename T>
    snw::future<T> make_ready_future(T value);

}

#include "future.hpp"
