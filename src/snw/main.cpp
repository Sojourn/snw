#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <memory>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "snw_util.h"
#include "snw_event.h"
#include "snw_lang.h"

#include "array.h"

#include "text_reader.h"
#include "lexer.h"

namespace snw {
    // Think about using a shadow stack. Currently references aren't stable across allocations or
    // suspends which isn't super useful.

    enum class object_type : uint16_t {
        nil,
        integer,
        symbol,
        string,
        cell,
    };

    class object_handle {
    public:
        object_handle() {
            set_type(object_type::nil);
            set_addr(0);
        }

        object_type type() const {
            return static_cast<object_type>(type_);
        }

        void set_type(object_type type) {
            type_ = static_cast<uint16_t>(type);
        }

        uint16_t addr() const {
            return (addr_ << addr_shift_);
        }

        void set_addr(uint16_t addr) {
            addr_ = (addr >> addr_shift_);
        }

        explicit operator bool() const {
            return (type() != object_type::nil) && (addr() != 0);
        }

    private:
        static constexpr uint32_t addr_shift_ = 3;

        uint16_t type_ : 3;
        uint16_t addr_ : 13;
        // uint16_t nonce_;
    };

    class object_heap {
    public:
        object_heap()
            : size_(0)
            , data_(new uint8_t[capacity_])
        {
            ::memset(data_.get(), 0, capacity_);

            setup();
        }

        void reset() {
            // zero previously allocated heap memory
            ::memset(data_.get(), 0, size_);
            size_ = 0;

            setup();
        }

        // allocate heap memory
        object_handle allocate(object_type type, size_t alloc_size) {
            assert(alloc_size > 0); // no zero-sized allocations

            alloc_size = align_up(alloc_size, alignment_);
            if (capacity_ < (size_ + alloc_size)) {
                throw std::runtime_error("object_heap allocation failed");
            }

            uint16_t addr = static_cast<uint16_t>(size_);
            size_ += alloc_size;

            object_handle handle;
            handle.set_type(type);
            handle.set_addr(addr);
            return handle;
        }

        // access heap memory
        template<typename T>
        T& access(object_handle handle) {
            assert(handle.addr() < size_);
            return *reinterpret_cast<T*>(&data_[handle.addr()]);
        }

        // access heap memory
        template<typename T>
        const T& access(object_handle handle) const {
            assert(handle.addr() < size_);
            return *reinterpret_cast<const T*>(&data_[handle.addr()]);
        }

    private:
        // initialize an empty heap
        void setup() {
            assert(size_ == 0);

            allocate(object_type::nil, 1);
        }

    private:
        static constexpr size_t    capacity_ = (1 << 16);
        static constexpr size_t    alignment_ = 8;

        size_t                     size_;
        std::unique_ptr<uint8_t[]> data_;
    };

    template<object_type type>
    class object;

    template<>
    class object<object_type::nil> {
    public:
        object(object_heap& heap, object_handle handle)
            : heap_(&heap)
            , handle_(handle)
        {
            assert(handle.type() == object_type::nil);
        }

        object(object_heap& heap)
            : heap_(&heap)
        {
            // collusion between the heap and nil objects
            handle_.set_type(object_type::nil);
            handle_.set_addr(0);
        }

    private:
        object_heap*  heap_;
        object_handle handle_;
    };

    template<>
    class object<object_type::integer> {
    public:
        object(object_heap& heap, object_handle handle)
            : heap_(&heap)
            , handle_(handle)
        {
            assert(handle.type() == object_type::integer);
        }

        object(object_heap& heap, int64_t value)
            : heap_(&heap)
            , handle_(heap.allocate(object_type::integer, sizeof(int64_t)))
        {
            set_value(value);
        }

        int64_t value() const {
            return heap_->access<int64_t>(handle_);
        }

        void set_value(int64_t value) {
            heap_->access<int64_t>(handle_) = value;
        }

    private:
        object_heap*  heap_;
        object_handle handle_;
    };

    template<>
    class object<object_type::symbol> {
        using name_t = varchar<16>;
    public:
        object(object_heap& heap, const char* str)
            : heap_(&heap)
            , handle_(heap.allocate(object_type::symbol, sizeof(name_t)))
        {
            new(&heap_->access<name_t>(handle_)) name_t(str);
        }        

        object(object_heap& heap, object_handle handle)
            : heap_(&heap)
            , handle_(handle)
        {
            assert(handle.type() == object_type::symbol);
        }

        const name_t& name() const {
            return heap_->access<name_t>(handle_);
        }

    private:
        object_heap*  heap_;
        object_handle handle_;
    };

    template<>
    class object<object_type::string> {
        using len_t = uint16_t;

        struct string {
            len_t len;
            char  buf[1];
        };

    public:
        object(object_heap& heap, const char* str)
            : heap_(&heap)
        {
            size_t len = strlen(str);
            if ((std::numeric_limits<len_t>::max() - 1) < len) {
                throw std::runtime_error("string is too long to be stored in heap");
            }

            handle_ = heap_->allocate(object_type::string, sizeof(len_t) + len + 1);

            auto& s = heap_->access<string>(handle_);
            s.len = static_cast<len_t>(len);
            memcpy(s.buf, str, len + 1);
        }

        object(object_heap& heap, object_handle handle)
            : heap_(&heap)
            , handle_(handle)
        {
            assert(handle.type() == object_type::string);
        }

        const char* c_str() const {
            return heap_->access<string>(handle_).buf;
        }

        size_t size() const {
            return heap_->access<string>(handle_).len;
        }

    private:
        object_heap*  heap_;
        object_handle handle_;
    };

    template<>
    class object<object_type::cell> {
        struct cell {
            // we've got enough extra storage to do interesting things here:
            //   - cache symbol lookups?
            //   - environment/closure?
            object_handle car;
            // object_handle car_tag;
            object_handle cdr;
            // object_handle cdr_tag;
        };
    public:
        object(object_heap& heap)
            : heap_(&heap)
            , handle_(heap.allocate(object_type::cell, sizeof(cell)))
        {
        }

        object(object_heap& heap, object_handle car, object_handle cdr)
            : heap_(&heap)
            , handle_(heap.allocate(object_type::cell, sizeof(cell)))
        {
            set_car(car);
            set_cdr(cdr);
        }

        // FIXME: explicit?
        object(object_heap& heap, object_handle handle)
            : heap_(&heap)
            , handle_(handle)
        {
            assert(handle.type() == object_type::cell);
        }

        object_handle car() const {
            return heap_->access<cell>(handle_).car;
        }

        void set_car(object_handle car) {
            heap_->access<cell>(handle_).car = car;
        }

        object_handle cdr() const {
            return heap_->access<cell>(handle_).cdr;
        }

        void set_cdr(object_handle cdr) {
            heap_->access<cell>(handle_).cdr = cdr;
        }

    private:
        object_heap*  heap_;
        object_handle handle_;
    };

    // utility functions for dealing with objects

    template<object_type type>
    object<type> object_cast(object_heap& heap, object_handle handle) {
        if (handle.type() != type) {
            throw std::runtime_error("invalid object_cast");
        }

        return object<type>(heap, handle);
    }

    template<object_type type, typename... Args>
    object<type> make_object(object_heap& heap, Args&&... args) {
        return object<type>(heap, std::forward<Args>(args)...);
    }

    template<typename... Args>
    object<object_type::symbol> make_symbol_object(object_heap& heap, Args&&... args) {
        return make_object<object_type::symbol>(heap, std::forward<Args>(args)...);
    }

    template<typename... Args>
    object<object_type::string> make_string_object(object_heap& heap, Args&&... args) {
        return make_object<object_type::string>(heap, std::forward<Args>(args)...);
    }
}

namespace snw {
    class virtual_machine {
    public:
        virtual_machine(int argc, const char** argv) {
            (void)argc;
            (void)argv;
        }
    };

}

struct dummy_parser {
    void open_file() {
        std::cout << "open_file" << std::endl;
    }
    void open_list() {
        std::cout << "open_list" << std::endl;
    }
    void integer(int64_t value) {
        std::cout << "integer:" << value << std::endl;
    }
    void string(const char* first, const char* last) {
        std::cout << "string:" << std::string(first, last-first) << std::endl;
    }
    void symbol(const char* first, const char* last) {
        std::cout << "symbol:" << std::string(first, last-first) << std::endl;
    }
    void comment(const char* first, const char* last) {
        std::cout << "comment:" << std::string(first, last-first) << std::endl;
    }
    void close_list() {
        std::cout << "list_file" << std::endl;
    }
    void close_file() {
        std::cout << "close_file" << std::endl;
    }
    void error(const snw::lexer_error& err) {
        std::cout << "error(msg:'" << err.msg << "' off:" << err.off << " column:" << err.column << " row:" << err.row << ")" << std::endl;
    }
};

int main(int argc, char** argv) {
    dummy_parser parser;
    snw::parse("(print (+ 1 2))", parser);
    snw::parse("$", parser);
    snw::parse("", parser);

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
