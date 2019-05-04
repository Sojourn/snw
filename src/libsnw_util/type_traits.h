#pragma once

namespace snw {

template<typename T>
struct invoke_result;

template<typename Result, typename... Args>
struct invoke_result<Result(Args...)> {
    using type = Result;
};

template<typename T>
using invoke_result_t = typename invoke_result<T>::type;

}
