#include <immintrin.h>
#include "bits.h"

#define VARCHAR_SIMD_ENABLED 1

template<int capacity_>
snw::varchar<capacity_>::varchar() : data_{} {
    clear();
}

template<int capacity_>
snw::varchar<capacity_>::varchar(const char* str) : data_{} {
    assign(str);
}

template<int capacity_>
snw::varchar<capacity_>::varchar(const char* str, int len) : data_{} {
    assign(str, len);
}

template<int capacity_>
snw::varchar<capacity_>& snw::varchar<capacity_>::operator=(const char* str) {
    assign(str);
}

template<int capacity_>
bool snw::varchar<capacity_>::empty() const {
    return data_[0] == '\0';
}

template<int capacity_>
int snw::varchar<capacity_>::size() const {
#if VARCHAR_SIMD_ENABLED
    __m128i null_vec = _mm_setzero_si128();

    for (int i = 0; i < capacity_; i += 16) {
        // load a chunk of the string and make a mask of null bytes in it
        int pad_mask = _mm_movemask_epi8(
            _mm_cmpeq_epi8(
                _mm_load_si128((const __m128i*)(data_ + i)),
                null_vec)
        );

        // return the index of the first null byte
        if (pad_mask) {
            return i + count_trailing_zeros(static_cast<uint32_t>(pad_mask));
        }
    }

    return capacity_;
#else
    int i = 0;
    for (; i < capacity_; ++i) {
        if (data_[i] == '\0') {
            break;
        }
    }

    return i;
#endif
}

template<int capacity_>
int snw::varchar<capacity_>::capacity() const {
    return capacity_;
}

template<int capacity_>
const char* snw::varchar<capacity_>::data() const {
    return data_;
}

template<int capacity_>
const char* snw::varchar<capacity_>::begin() const {
    return data_;
}

template<int capacity_>
const char* snw::varchar<capacity_>::end() const {
    return data_ + size();
}

template<int capacity_>
const char& snw::varchar<capacity_>::at(int index) const {
    if ((index < 0) || (index >= capacity_)) {
        throw std::runtime_error("varchar index out of bounds");
    }

    return data_[index];
}

template<int capacity_>
const char& snw::varchar<capacity_>::operator[](int index) const {
    return data_[index];
}

template<int capacity_>
void snw::varchar<capacity_>::clear() {
    for (int i = 0; i < capacity_; ++i) {
        data_[i] = '\0';
    }
}

template<int capacity_>
void snw::varchar<capacity_>::assign(const char* str) {
    for (int i = 0; i < capacity_; ++i) {
        data_[i] = *str;
        if (*str != '\0') {
            str++;
        }
        else {
            // keep assigning '\0' from str
        }
    }

    if (*str != '\0') {
        throw std::runtime_error("varchar buffer overflow");
    }
}

template<int capacity_>
void snw::varchar<capacity_>::assign(const char* str, int len) {
    if (len > capacity_) {
        throw std::runtime_error("varchar buffer overflow");
    }

    int i = 0;
    for (; i < len; ++i) {
        data_[i] = str[i];
    }

    for (; i < capacity_; ++i) {
        data_[i] = '\0';
    }
}

template<int capacity_>
int snw::compare(const varchar<capacity_>& lhs, const varchar<capacity_>& rhs) {
    int rc = 0;
    for (int i = 0; i < capacity_; ++i) {
        rc = static_cast<int>(lhs[i]) - static_cast<int>(rhs[i]);
        if (rc) {
            break; // found a character that wasn't equal
        }
    }

    return rc;
}
