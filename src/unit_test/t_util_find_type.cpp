// #include "catch.hpp"
#include "find_type.h"

namespace {
struct A {};
struct B {};
struct C {};
};

static_assert(snw::find_type<A>::value == -1, "");
static_assert(snw::find_type<B>::value == -1, "");
static_assert(snw::find_type<C>::value == -1, "");

static_assert(snw::find_type<A, A>::value == 0, "");
static_assert(snw::find_type<B, A>::value == -1, "");
static_assert(snw::find_type<C, A>::value == -1, "");

static_assert(snw::find_type<A, A, B>::value == 0, "");
static_assert(snw::find_type<B, A, B>::value == 1, "");
static_assert(snw::find_type<C, A, B>::value == -1, "");

static_assert(snw::find_type<A, A, B, C>::value == 0, "");
static_assert(snw::find_type<B, A, B, C>::value == 1, "");
static_assert(snw::find_type<C, A, B, C>::value == 2, "");
