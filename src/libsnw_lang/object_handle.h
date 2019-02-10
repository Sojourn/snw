#pragma once

#include <cstring>
#include "intrusive_list.h"

namespace snw {

class object_heap;

enum class object_type : uint16_t {
    nil,
    boolean,
    integer,
    symbol,
    string,
    cell,
};

struct raw_object_handle {
    uint16_t type : 3;
    uint16_t addr : 13;
};

class object_handle {
    template<typename T, snw::intrusive_list_node T::*member_node>
    friend class snw::intrusive_list;
    friend class object_heap;

    template<object_type>
    friend class object;

public:
    // FIXME: there's a bug in intrusive_list that causes a crash without this
    object_handle(object_handle&& other)
        : heap_(other.heap_)
        , handle_(other.handle_)
    {
        other.heap_ = nullptr;
        memset(&other.handle_, 0, sizeof(handle_));

        attach();
        other.detach(); 
    }

    object_handle(const object_handle&) = delete;

    // FIXME: there's a bug in intrusive_list that causes a crash without this
    object_handle& operator=(object_handle&& rhs) {
        if (this != &rhs) {
            detach();

            heap_ = rhs.heap_;
            handle_ = rhs.handle_;

            rhs.heap_ = nullptr;
            memset(&rhs.handle_, 0, sizeof(handle_));

            attach();
            rhs.detach();
        }
        
        return *this;
    }

    object_handle& operator=(const object_handle&) = delete;

    object_heap& heap() {
        return *heap_;
    }

    const object_heap& heap() const {
        return *heap_;
    }

    uint16_t addr() const {
        return static_cast<uint16_t>(handle_.addr) << 3;
    }

    object_type type() const {
        return static_cast<object_type>(handle_.type);
    }

protected:
    object_handle(object_heap& heap, raw_object_handle handle)
        : heap_(&heap)
        , handle_(handle)
    {
        attach();
    }

    object_handle(object_heap& heap, object_type type, uint16_t addr)
        : heap_(&heap)
    {
        set(type, addr);
        attach();
    }

    void set(object_type type, uint16_t addr) {
        handle_.type = static_cast<uint16_t>(type);
        handle_.addr = addr >> 3;
    }

    void set_addr(uint16_t addr) {
        set(type(), addr);
    }

    void set_type(object_type type) {
        set(type, addr());
    }

    raw_object_handle raw_handle() const {
        return handle_;
    }

protected:
    object_heap*             heap_;
    raw_object_handle        handle_;

private:
    snw::intrusive_list_node ref_;

    void attach();
    void detach();
};

}
