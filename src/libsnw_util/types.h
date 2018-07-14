#pragma once

#include <cstdint>
#include <cstddef>

#include <immintrin.h>

namespace snw {
    using int128_t = __m128i;
    using int256_t = __m256i;
}
