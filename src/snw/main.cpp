#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <memory>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "snw_util.h"
#include "snw_event.h"

// #include "object.h"
// #include "object_heap.h"

using namespace snw;

using atomic_uint16_t = std::atomic<uint16_t>;
using atomic_uint32_t = std::atomic<uint32_t>;
using atomic_uint64_t = std::atomic<uint64_t>;
using atomic_size_t   = std::atomic<size_t>;

using symbol = varchar<16>;

enum class object_type : uint8_t {
    nil,
    boolean,
    integer,
    symbol,
    string,
    bytes,
    cell,
};

inline const char* object_type_name(object_type type) {
    switch (type) {
    case object_type::nil:
        return "nil";
    case object_type::boolean:
        return "boolean";
    case object_type::integer:
        return "integer";
    case object_type::symbol:
        return "symbol";
    case object_type::string:
        return "string";
    case object_type::bytes:
        return "bytes";
    case object_type::cell:
        return "cell";
    default:
        return "?";
    }
}

struct object_ref {
    object_type type;
    bool        is_indirect : 1;

    union {
        uint16_t address;
        bool     boolean;
        int16_t  integer;
    } value;

    object_ref() {
        memset(this, 0, sizeof(*this));
    }
};

inline uint32_t encode_object_ref(object_ref ref) {
    uint32_t result;
    memcpy(&result, &ref, sizeof(result));
    return result;
}

inline object_ref decode_object_ref(uint32_t value) {
    object_ref ref;
    memcpy(&ref, &value, sizeof(ref));
    return ref;
}

inline uint32_t hash_object_ref(object_ref ref) {
    return hash32(encode_object_ref(ref));
}

inline std::ostream& operator<<(std::ostream& out, const object_ref& ref) {
    out << "object_ref{";
    out << "type:" << object_type_name(ref.type);
    if (ref.is_indirect) {
        out << " address:" << ref.value.address; // TODO: format as hex
    }
    else {
        out << " value:";
        switch (ref.type) {
        case object_type::boolean:
            out << (ref.value.boolean ? "true" : "false");
            break;
        case object_type::integer:
            out << ref.value.integer;
            break;
        case object_type::string:
            out << "\"\"";
            break;
        case object_type::bytes:
            out << "[]";
            break;
        case object_type::nil:
        default:
            break;
        }
    }
    out << "}";
    return out;
}

class object_stack {
public:
    object_stack()
        : size_(0)
    {
    }

    void push(object_ref ref) {
        if (size_ == capacity_) {
            throw std::runtime_error("object stack overflow");
        }

        refs_[size_++] = encode_object_ref(ref);
    }

    object_ref pop() {
        if (size_ == 0) {
            throw std::runtime_error("object stack underflow");
        }

        return decode_object_ref(refs_[--size_]);
    }

    object_ref operator[](size_t index) const {
        if (size_ < index) {
            throw std::runtime_error("object stack index out of bounds");
        }

        return decode_object_ref(refs_[size_ - index - 1]);
    }

private:
    static constexpr size_t capacity_ = 1024;

    size_t   size_;
    uint32_t refs_[capacity_];
};

struct cell_object {
    uint32_t car;
    uint32_t cdr;
};

struct string_object {
    uint16_t len;
    char     str[0];
};
static_assert(sizeof(string_object) == sizeof(uint16_t), "unexpected string_object size");

struct bytes_object {
    uint16_t len;
    uint8_t  buf[0];
};
static_assert(sizeof(bytes_object) == sizeof(uint16_t), "unexpected bytes_object size");

class object_heap {
public:
    object_heap()
        : size_(0)
        , old_size_(0)
    {
    }

    object_ref new_nil() {
        object_ref ref;
        ref.type = object_type::nil;
        return ref;
    }

    object_ref new_boolean(bool value) {
        object_ref ref;
        ref.type = object_type::boolean;
        ref.value.boolean = value;
        return ref;
    }

    object_ref new_integer(int64_t value) {
        object_ref ref;
        ref.type = object_type::integer;
        if ((std::numeric_limits<int16_t>::min() <= value) && (value <= std::numeric_limits<int16_t>::max())) {
            ref.value.integer = static_cast<int16_t>(value);
        }
        else {
            check_free_space(sizeof(value));

            ref.is_indirect = true;
            ref.value.address = size_;

            access<int64_t>(ref) = value;

            size_ += sizeof(value);
        }

        return ref;
    }

    object_ref new_symbol(const symbol& value) {
        check_free_space(sizeof(value));

        object_ref ref;
        ref.type = object_type::symbol;
        ref.is_indirect = true;
        ref.value.address = size_;

        access<symbol>(ref) = value;

        size_ += sizeof(value);
        return ref;
    }

    object_ref new_string(const char* str) {
        return new_string(str, str + strlen(str));
    }

    object_ref new_string(const char* first, const char* last) {
        object_ref ref;
        ref.type = object_type::string;
        if (first != last) {
            size_t str_len = last - first;
            size_t alloc_size = align_up(sizeof(string_object) + str_len + 1, alignment_);
            check_free_space(alloc_size);

            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            auto& obj = access<string_object>(ref);
            obj.len = str_len;
            memcpy(obj.str, first, str_len);
            obj.str[str_len] = '\0';

            size_ += alloc_size;
        }

        return ref;
    }

    object_ref new_bytes(const uint8_t* first, const uint8_t* last) {
        object_ref ref;
        ref.type = object_type::bytes;
        if (first != last) {
            size_t buf_len = last - first;
            size_t alloc_size = align_up(sizeof(bytes_object) + buf_len, alignment_);
            check_free_space(alloc_size);

            ref.is_indirect = true;
            ref.value.address = static_cast<uint16_t>(size_);

            auto& obj = access<bytes_object>(ref);
            obj.len = buf_len;
            memcpy(obj.buf, first, buf_len);

            size_ += alloc_size;
        }

        return ref;
    }

    object_ref new_cell(object_ref car, object_ref cdr) {
        check_free_space(sizeof(cell_object));

        object_ref ref;
        ref.type = object_type::cell;
        ref.is_indirect = true;
        ref.value.address = static_cast<uint16_t>(size_);

        cell_object cell;
        cell.car = encode_object_ref(car);
        cell.cdr = encode_object_ref(cdr);
        access<cell_object>(ref) = cell;

        size_ += sizeof(cell_object);
        return ref;
    }

public:
    void begin() {
        assert(old_size_ == 0);
        old_size_ = size_;
    }

    void commit() {
        old_size_ = 0;
    }

    void rollback() {
        size_ = old_size_;
        old_size_ = 0;
    }

private:
    void check_free_space(size_t object_size) const {
        if (capacity_ < (size_ + object_size)) {
            // TODO: specialized exception type for this
            throw std::runtime_error("object heap out of memory");
        }
    }

    template<typename T>
    T& access(object_ref ref) {
        assert(ref.is_indirect);
        return *reinterpret_cast<T*>(&data_[ref.value.address]);
    }

    template<typename T>
    const T& access(object_ref ref) const {
        assert(ref.is_indirect);
        return *reinterpret_cast<const T*>(&data_[ref.value.address]);
    }

private:
    static constexpr size_t capacity_ = 1 << 16;
    static constexpr size_t alignment_ = 8;

    size_t  size_;
    size_t  old_size_;
    uint8_t data_[capacity_];
};

#if 0

struct nil_object {
};

struct string_object {
    uint16_t len;
    char     str[0];
};
static_assert(sizeof(string_object) == sizeof(uint16_t), "Bad string_object packing");

// struct bytes_object {
//     uint16_t len;
//     uint8_t  buf[0];
// };
// static_assert(sizeof(bytes_object) == sizeof(uint16_t), "Bad bytes_object packing");

struct cell_object {
    object_ref car;
    object_ref cdr;
};

inline bool is_small_integer(int64_t value) {
    return (std::numeric_limits<int16_t>::min() <= value) && (value <= std::numeric_limits<int16_t>::max());
}

class object_heap {
public:
    object_heap() {
        clear();
    }

    object_ref new_nil() {
        object_ref ref;
        memset(&ref, 0, sizeof(ref));
        return ref;
    }

    object_ref new_boolean(bool value) {
        object_ref ref;
        ref.type = boolean_object_type;
        ref.boolean = value;
        return ref;
    }

    object_ref new_integer(int64_t value) {
        object_ref ref;
        ref.type = integer_object_type;
        if (is_small_integer(value)) {
            ref.type |= small_object_flag;
            ref.integer = static_cast<int16_t>(value);
        }
        else {
            size_t len = sizeof(value);
            ref.address = prepare_alloc(&len);
            deref<int64_t>(ref) = value;
            commit_alloc(len);
        }

        return ref;
    }

    object_ref new_symbol(const symbol& sym) {
        object_ref ref;
        ref.type = symbol_object_type;

        size_t len = sizeof(symbol);
        ref.address = prepare_alloc(&len);
        deref<symbol>(ref) = sym;
        commit_alloc(len);
        
        return ref;
    }

    object_ref new_string(const char* str) {
        return new_string(str, str + strlen(str));
    }

    object_ref new_string(const char* first, const char* last) {
        object_ref ref;
        ref.type = string_object_type;

        size_t str_len = last - first;
        size_t alloc_len = sizeof(string_object) + str_len + 1;
        ref.address = prepare_alloc(&alloc_len);

        auto& object = deref<string_object>(ref);
        object.len = str_len;
        memcpy(object.str, first, str_len + 1);
        
        commit_alloc(alloc_len);
        return ref;
    }

    // object_ref new_bytes(const uint8_t* buf, size_t len) {
    // }

    object_ref new_cell(object_ref car, object_ref cdr) {
        object_ref ref;
        ref.type = cell_object_type;

        size_t len = sizeof(cell_object);
        ref.address = prepare_alloc(&len);

        auto& object = deref<cell_object>(ref);
        object.car = car;
        object.cdr = cdr;

        commit_alloc(len);
        return ref;
    }

    nil_object load_nil(object_ref ref) {
        nil_object nil;
        return nil;
    }

    bool load_boolean(object_ref ref) {
        return ref.boolean;
    }

    int64_t load_integer(object_ref ref) {
        if (ref.type & small_object_flag) {
            return ref.integer;
        }
        else {
            return deref<int64_t>(ref);
        }
    }

    const symbol& load_symbol(object_ref ref) {
        return deref<symbol>(ref);
    }

    const string_object& load_string(object_ref ref) {
        return deref<string_object>(ref);
    }

    // const bytes_object& load_bytes(object_ref ref) {
    //     return deref<bytes_object>(ref);
    // }

    const cell_object& load_cell(object_ref ref) {
        return deref<cell_object>(ref);
    }

    void clear() {
        size_.store(0);
        local_size_ = 0;
    }

private:
    uint16_t prepare_alloc(size_t* alloc_size) {
        *alloc_size = align_up(*alloc_size, 8);
        if ((local_size_ + *alloc_size) > capacity_) {
            throw std::runtime_error("out of memory");
        }

        return local_size_;
    }

    void commit_alloc(size_t alloc_size) {
        local_size_ += alloc_size;
        size_.store(static_cast<uint32_t>(local_size_));
    }

    void rollback_alloc(size_t alloc_size) {
        memset(&data_[local_size_], 0, alloc_size);
    }

    template<typename T>
    T& deref(object_ref ref) {
        return *reinterpret_cast<T*>(&data_[ref.address]);
    }

    template<typename T>
    const T& deref(object_ref ref) const {
        return *reinterpret_cast<const T*>(&data_[ref.address]);
    }

private:
    static constexpr size_t capacity_ = 1 << 16;

    atomic_size_t size_;
    // TODO: pad
    size_t        local_size_;
    uint8_t       data_[capacity_];
};

class object_stack {
public:
    object_stack() = default;

    void push(object_ref ref) {
        uint32_t value;
        memcpy(&value, &ref, sizeof(value));

        size_t size = size_.load();
        size_t new_size = size + 1;
        if (new_size == capacity_) {
            throw std::runtime_error("stack overflow");
        }

        refs_[size].store(value);
        size_.store(new_size);
    }

    void pop() {
        if (size_ == 0) {
            throw std::runtime_error("stack underflow");
        }

        --size_;
    }

    size_t size() const {
        return size_.load();
    }

    object_ref operator[](size_t index) {
        assert(index < size_);

        uint32_t value = refs_[index].load();
        object_ref ref;
        memcpy(&ref, &value, sizeof(ref));

        return ref;
    }

private:
    static constexpr size_t capacity_ = 1024;

    atomic_uint32_t size_;
    atomic_uint32_t refs_[capacity_];
};

class process_memory {
public:
    object_heap  heap;
    object_stack stack;
};

#endif

int main(int argc, char** argv) {
    uint8_t buf[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

    object_heap heap;
    {
        std::cout << heap.new_nil() << std::endl;
        std::cout << heap.new_boolean(true) << std::endl;
        std::cout << heap.new_boolean(false) << std::endl;
        std::cout << heap.new_symbol("Hello-World") << std::endl;
        std::cout << heap.new_integer(13) << std::endl;
        std::cout << heap.new_integer(1 << 20) << std::endl;
        std::cout << heap.new_string("") << std::endl;
        std::cout << heap.new_string("hello, world") << std::endl;
        std::cout << heap.new_bytes(buf, buf) << std::endl;
        std::cout << heap.new_bytes(buf, buf + sizeof(buf)) << std::endl;
    }

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
