#pragma once

namespace snw {

struct page {
    alignas(4096) uint8_t data[4096];
};
static_assert(alignof(page) == 4096, "");
static_assert(sizeof(page) == 4096, "");

}
