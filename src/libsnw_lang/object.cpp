#include "object.h"

using namespace snw;

const char* snw::object_type_name(object_type type) {
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

std::ostream& object_ref::operator<<(std::ostream& out, const object_ref& ref) {
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
