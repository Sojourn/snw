#pragma once

#include <ostream>
#include <cstdint>
#include <cstring>
#include "varchar.h"
#include "hash.h"

namespace snw {

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

const char* object_type_name(object_type type);

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

    uint32_t encode() const {
        uint32_t result;
        memcpy(&result, this, sizeof(result));
        return result;
    }

    static object_ref decode(uint32_t value) {
        object_ref result;
        memcpy(&result, &value, sizeof(result));
        return result;
    }

    uint32_t hash() const {
        return hash32(encode());
    }

    friend std::ostream& operator<<(std::ostream& out, const object_ref& ref);
};

struct cell_object {
    object_ref car;
    object_ref cdr;
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

}
