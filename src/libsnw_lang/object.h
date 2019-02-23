#pragma once

#include <ostream>
#include <cstdint>
#include <cstring>
#include "object_ref.h"
#include "object_heap.h"

namespace snw {

template<object_type type>
class object;

template<>
class object<object_type::nil> {
public:
    object(const object_heap& heap, object_ref ref)
        : heap_(heap)
        , ref_(ref)
    {
        assert(ref_.type == object_type::nil);
    }

    const object_heap& heap() const {
        return heap_;
    }

    object_ref ref() const {
        return ref_;
    }

private:
    const object_heap& heap_;
    const object_ref   ref_;
};

template<>
class object<object_type::integer> {
public:
    object(const object_heap& heap, object_ref ref)
        : heap_(heap)
        , ref_(ref)
    {
        assert(ref_.type == object_type::integer);
    }

    const object_heap& heap() const {
        return heap_;
    }

    object_ref ref() const {
        return ref_;
    }

    int64_t value() const {
        if (ref_.is_indirect) {
            return ref_.value.integer;
        }
        else {
            auto& repr = heap_.deref_integer(ref_);
            return repr.value;
        }
    }

private:
    integer_repr repr() const {
        if (ref_.is_indirect) {
            integer_repr repr;
            repr.value = ref_.value.integer;
            return repr;
        }
        else {
            return heap_.deref_integer(ref_);
        }
    }

    const object_heap& heap_;
    const object_ref   ref_;
};

template<>
class object<object_type::symbol> {
public:
    object(const object_heap& heap, object_ref ref)
        : heap_(heap)
        , ref_(ref)
    {
        assert(ref_.type == object_type::symbol);
    }

    const object_heap& heap() const {
        return heap_;
    }

    object_ref ref() const {
        return ref_;
    }

    const symbol& name() const {
        return repr().name;
    }

private:
    const symbol_repr& repr() const {
        if (ref_.is_indirect) {
            static const symbol_repr repr;
            return repr;
        }
        else {
            return heap_.deref_symbol(ref_);
        }
    }

    const object_heap& heap_;
    const object_ref   ref_;
};

template<>
class object<object_type::string> {
public:
    object(const object_heap& heap, object_ref ref)
        : heap_(heap)
        , ref_(ref)
    {
        assert(ref_.type == object_type::string);
    }

    const object_heap& heap() const {
        return heap_;
    }

    object_ref ref() const {
        return ref_;
    }

    bool empty() const {
        return ref_.is_indirect;
    }

    size_t size() const {
        return repr().len;
    }

    const char* data() const {
        return repr().str;
    }

    const char* begin() const {
        return data();
    }

    const char* end() const {
        return data() + size();
    }

    const char* c_str() const {
        return data();
    }

private:
    const string_repr& repr() const {
        if (ref_.is_indirect) {
            static const string_repr repr;
            return repr;
        }
        else {
            return heap_.deref_string(ref_);
        }
    }

    const object_heap& heap_;
    const object_ref   ref_;
};

template<>
class object<object_type::bytes> {
public:
    object(const object_heap& heap, object_ref ref)
        : heap_(heap)
        , ref_(ref)
    {
        assert(ref_.type == object_type::bytes);
    }

    const object_heap& heap() const {
        return heap_;
    }

    object_ref ref() const {
        return ref_;
    }

    bool empty() const {
        return ref_.is_indirect;
    }

    size_t size() const {
        return repr().len;
    }

    const uint8_t* data() const {
        return repr().buf;
    }

    const uint8_t* begin() const {
        return data();
    }

    const uint8_t* end() const {
        return data() + size();
    }

private:
    const bytes_repr& repr() const {
        if (ref_.is_indirect) {
            static const bytes_repr repr;
            return repr;
        }
        else {
            return heap_.deref_bytes(ref_);
        }
    }

    const object_heap& heap_;
    const object_ref   ref_;
};

template<>
class object<object_type::cell> {
public:
    object(const object_heap& heap, object_ref ref)
        : heap_(heap)
        , ref_(ref)
    {
        assert(ref_.type == object_type::cell);
    }

    const object_heap& heap() const {
        return heap_;
    }

    object_ref ref() const {
        return ref_;
    }

    object_ref car() const {
        return repr().car;
    }

    object_ref cdr() const {
        return repr().cdr;
    }

private:
    const cell_repr& repr() const {
        if (ref_.is_indirect) {
            static const cell_repr repr;
            return repr;
        }
        else {
            return heap_.deref_cell(ref_);
        }
    }

    const object_heap& heap_;
    const object_ref   ref_;
};

using nil_object = object<object_type::nil>;
using integer_object = object<object_type::integer>;
using symbol_object = object<object_type::symbol>;
using string_object = object<object_type::string>;
using bytes_object = object<object_type::bytes>;
using cell_object = object<object_type::cell>;

template<object_type type>
object<type> unsafe_object_cast(const object_heap& heap, object_ref ref) {
    return object<type>(heap, ref);
}

template<object_type type>
object<type> object_cast(const object_heap& heap, object_ref ref) {
    if (ref.type != type) {
        throw std::runtime_error("invalid object cast");
    }

    return unsafe_object_cast<type>(heap, ref);
}

template<typename Handler>
void dispatch(const object_heap& heap, object_ref ref, Handler&& handler) {
    switch (ref.type) {
    case object_type::nil:
        handler(unsafe_object_cast<object_type::nil>(heap, ref));
        break;

    case object_type::integer:
        handler(unsafe_object_cast<object_type::integer>(heap, ref));
        break;

    case object_type::symbol:
        handler(unsafe_object_cast<object_type::symbol>(heap, ref));
        break;

    case object_type::string:
        handler(unsafe_object_cast<object_type::string>(heap, ref));
        break;

    case object_type::bytes:
        handler(unsafe_object_cast<object_type::bytes>(heap, ref));
        break;

    case object_type::cell:
        handler(unsafe_object_cast<object_type::cell>(heap, ref));
        break;
    }
}

enum class visit_order {
    pre_order,
    in_order,
    post_order,
};

template<typename Handler>
void visit_object(const object_heap& heap, object_ref ref, Handler&& handler, visit_order order = visit_order::pre_order) {
    struct {
        const object_heap& heap;
        Handler&           handler;
        visit_order        order;

        void operator()(nil_object o) {
            handler(o);
        }

        void operator()(integer_object o) {
            handler(o);
        }

        void operator()(string_object o) {
            handler(o);
        }

        void operator()(bytes_object o) {
            handler(o);
        }

        void operator()(cell_object o) {
            switch (order) {
            case visit_order::pre_order:
                handler(o);
                visit_object(heap, o.car(), handler);
                visit_object(heap, o.cdr(), handler);
                break;

            case visit_order::in_order:
                visit_object(heap, o.car(), handler);
                handler(o);
                visit_object(heap, o.cdr(), handler);
                break;

            case visit_order::post_order:
                visit_object(heap, o.car(), handler);
                visit_object(heap, o.cdr(), handler);
                handler(o);
                break;
            }
        }

    } dispatcher = {
        heap,
        handler,
        order
    };

    dispatcher(heap, ref, dispatcher);
}

}
