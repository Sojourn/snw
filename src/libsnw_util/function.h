#pragma once

namespace snw {

    template<typename T>
    class function;

    template<typename Result, typename... Args>
    class function<Result(Args...)> {
    public:
        function();
        function(function&& other);
        function(const function&) = delete;

        function(Result(*fn)(Args...));

        template<typename T>
        function(T* object, Result(T::*mem_fn)(Args...));

        template<typename T>
        function(const T* object, Result(T::*mem_fn)(Args...) const); // TODO?

        template<typename Functor>
        function(Functor&& functor);

        ~function();

        function& operator=(function&& rhs);
        function& operator=(const function&) = delete;

        function& operator=(Result(*fn)(Args...));

        template<typename Functor>
        function& operator=(Functor&& functor);

        Result operator()(Args... args);
        
        Result operator()(Args... args) const; // TODO?

        explicit operator bool() const;
    };

}
