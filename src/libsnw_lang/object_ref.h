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

    // struct {
    //     bool is_indirect : 1;
    // } flags;

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

    friend bool operator==(const object_ref& l, const object_ref& r) {
        return (memcmp(&l, &r, sizeof(l)) == 0);
    }

    friend bool operator!=(const object_ref& l, const object_ref& r) {
        return (memcmp(&l, &r, sizeof(l)) == 0);
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

struct nil_repr {
};

struct integer_repr {
    int64_t value = 0;
};

using symbol = varchar<16>;

struct symbol_repr {
    symbol name;
};

struct string_repr {
    uint16_t len = 0;
    char     str[0] = {};
};

struct bytes_repr {
    uint16_t len = 0;
    uint8_t  buf[0] = {};
};

struct cell_repr {
    object_ref car;
    object_ref cdr;
};

}