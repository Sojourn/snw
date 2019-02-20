#include "object.h"

using namespace snw;

const char* snw::object_type_name(object_type type) {
    switch (type) {
    case object_type::nil:
        return "nil";
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
