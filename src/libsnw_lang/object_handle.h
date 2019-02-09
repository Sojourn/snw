#pragma once

#include "intrusive_list.h"

namespace snw {

enum class object_type : uint16_t {
    nil,
    // boolean,
    integer,
    symbol,
    string,
    cell,
};

// struct compressed_object_handle {
//     uint16_t type_ : 3;
//     uint16_t addr_ : 13;
    
//     object_type type() const {
//         return static_cast<object_type>(type_);
//     }

//     void set_type(object_type type) {
//         type_ = static_cast<uint16_t>(type);
//     }

//     uint16_t addr() const {
//         return (addr_ << addr_shift_);
//     }

//     void set_addr(uint16_t addr) {
//         addr_ = (addr >> addr_shift_);
//     }
// };

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

}