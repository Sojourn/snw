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
    public:
        future();
        future(future&& other);
        future(const future&) = delete;
        ~future();

        future& operator=(future&& rhs);
        future& operator=(const future&) = delete;

    private:
        future(promise<T>* promise);

        void attach(promise<T>* promise);
        void detach();

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
        class future;

        template<typename T_>
        friend struct future_promise;
    public:
        promise();
        promise(promise&& other);
        promise(const promise&) = delete;
        ~promise();

        promise& operator=(promise&& rhs);
        promise& operator=(const promise&) = delete;

    private:
        promise(future<T>* future);

        future<T>* future_;
    };

    template<typename T>
    struct future_promise {
        future<T> future;
        promise<T> promise;

        future_promise(): future(&promise), promise(&future) {}
    };

}

#include "future.hpp"
