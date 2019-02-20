#pragma once

#include <ostream>
#include <cstdint>
#include <cstring>
#include "varchar.h"
#include "hash.h"

namespace snw {

enum class object_type : uint8_t {
    nil,
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

    friend std::ostream& operator<<(std::ostream& out, const object_ref& ref) {
        out << "object_ref{";
        out << "type:" << object_type_name(ref.type);
        if (ref.is_indirect) {
            out << " address:" << ref.value.address; // TODO: format as hex
        }
        else {
            out << " value:";
            switch (ref.type) {
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
};

// TODO: symbol_object
using symbol = varchar<16>;

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

struct cell_object {
    object_ref car;
    object_ref cdr;
};

}
