#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <bitset>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "snw_util.h"
#include "snw_event.h"

#include "array.h"

// rename to atom-type?
enum class object_type : uint16_t {
    nil,
    integer,
    string,
    cell,
};

struct object_header {
    object_type type;
    uint16_t    size;
};

struct nil_object {
    object_header header;
};

struct cell_object {
    object_header header;
    uint16_t      car;
    uint16_t      cdr;
};

struct integer_object {
    object_header header;
    int64_t       value;
};

struct string_object {
    object_header header;
    char          buffer[1];
};

class task_heap;

class handle {
public:
    handle(task_heap& heap, size_t addr)
        : heap_(&heap)
        , addr_(addr)
    {
    }

    task_heap& heap() {
        return *heap_;
    }

    const task_heap& heap() const {
        return *heap_;
    }

    size_t address() const {
        return addr_;
    }

    object_type type() const {
        object_type type;
        read_field(offsetof(object_header, type), &type);
        return type;
    }

    size_t size() const {
        uint16_t size;
        read_field(offsetof(object_header, size), &size);
        return size;
    }

    handle next() {
        return handle(heap(), addr_ + size());
    }

    explicit operator bool() const {
        return addr_ < (1 << 16);
    }

protected:
    template<typename T>
    void read_field(size_t offset, T* value) const;

    template<typename T>
    void write_field(size_t offset, const T& value);

private:
    task_heap* heap_;
    size_t     addr_;
};

template<object_type type>
class object;

template<>
class object<object_type::nil> : public handle {
public:
    object(task_heap& heap)
        : handle(heap, 0)
    {
        assert(type() == object_type::nil);
    }

    object(handle hnd)
        : handle(hnd)
    {
        assert(type() == object_type::nil);
    }
};

template<>
class object<object_type::integer> : public handle {
public:
    object(handle hnd)
        : handle(hnd)
    {
        assert(type() == object_type::integer);
    }

    object(task_heap& heap, size_t addr)
        : handle(heap, addr)
    {
        assert(type() == object_type::integer);
    }

    int64_t value() const {
        int64_t value;
        read_field(offsetof(integer_object, value), &value);
        return value;
    }

    void set_value(int64_t value) {
        write_field(offsetof(integer_object, value), value);
    }
};

template<>
class object<object_type::cell> : public handle {
public:
    object(handle hnd)
        : handle(hnd)
    {
        assert(type() == object_type::cell);
    }

    object(task_heap& heap, size_t addr)
        : handle(heap, addr)
    {
        assert(type() == object_type::cell);
    }

    handle car() {
        uint16_t addr;
        read_field(offsetof(cell_object, car), &addr);
        return handle(heap(), addr);
    }

    void set_car(handle hnd) {
        assert(&heap() == &hnd.heap());
        uint16_t addr = static_cast<uint16_t>(hnd.address());
        write_field(offsetof(cell_object, car), addr);
    }

    handle cdr() {
        uint16_t addr;
        read_field(offsetof(cell_object, cdr), &addr);
        return handle(heap(), addr);
    }

    void set_cdr(handle hnd) {
        assert(&heap() == &hnd.heap());
        uint16_t addr = static_cast<uint16_t>(hnd.address());
        write_field(offsetof(cell_object, cdr), addr);
    }
};

class task_heap {
public:
    task_heap() {
        allocate(object_type::nil, sizeof(nil_object));
    }

    handle allocate(object_type type, size_t size) {
        size_t aligned_size = snw::align_up(size, alignment_);
        if ((capacity_ - size_) < aligned_size) {
            throw std::runtime_error("Out of memory");
        }

        size_t addr = size_;
        size_ += aligned_size;

        object_header header;
        header.type = type;
        header.size = aligned_size;

        memset(data() + addr, 0, aligned_size);
        memcpy(data() + addr, &header, sizeof(header));

        return handle(*this, addr);
    }

    handle begin() {
        return handle(*this, 0);
    }

    handle end() {
        return handle(*this, size_);
    }

    uint8_t* data() {
        return data_.data();
    }

    const uint8_t* data() const {
        return data_.data();
    }

private:
    friend class handle;

    void read(size_t addr, void* buf, size_t len) const {
        check_bounds(addr, len);
        memcpy(buf, data() + addr, len);
    }

    void write(size_t addr, const void* buf, size_t len) {
        check_bounds(addr, len);
        memcpy(data() + addr, buf, len);
    }

private:
    void check_bounds(size_t addr, size_t len) const {
        size_t outer_beg = 0;
        size_t outer_end = capacity_;
        size_t inner_beg = addr;
        size_t inner_end = addr + len;

        if ((inner_beg < outer_beg) || (outer_end < inner_end)) {
            throw std::runtime_error("invalid bounds");
        }
    }

private:
    static constexpr size_t        capacity_ = 1 << 16;
    static constexpr size_t        alignment_ = 8;
    size_t                         size_ = 0;
    std::array<uint8_t, capacity_> data_;
};

template<typename T>
void handle::read_field(size_t offset, T* value) const {
    heap_->read(addr_ + offset, value, sizeof(*value));
}

template<typename T>
void handle::write_field(size_t offset, const T& value) {
    heap_->write(addr_ + offset, &value, sizeof(value));
}

template<object_type type, typename... Args>
object<type> make_object(task_heap& heap, Args&&... args);

template<>
object<object_type::nil> make_object(task_heap& heap) {
    return object<object_type::nil>(heap);
}

template<>
object<object_type::integer> make_object(task_heap& heap) {
    return object<object_type::integer>(
        heap.allocate(object_type::integer, sizeof(integer_object)));
}

template<>
object<object_type::cell> make_object(task_heap& heap) {
    return object<object_type::cell>(
        heap.allocate(object_type::cell, sizeof(cell_object)));
}

template<object_type type>
object<type> object_cast(handle hnd) {
    assert(hnd);
    assert(hnd.type() == type);
    return object<type>(hnd);
}

int main(int argc, char** argv) {
    task_heap heap;

    auto car = make_object<object_type::integer>(heap);
    car.set_value(13);

    auto cdr = make_object<object_type::nil>(heap);

    auto cell = make_object<object_type::cell>(heap);
    cell.set_car(car);
    cell.set_cdr(cdr);

    auto cell_hnd = static_cast<handle>(cell);
    auto cell_obj = object_cast<object_type::cell>(cell_hnd);

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
