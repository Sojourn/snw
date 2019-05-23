#include <iostream>
#include <memory>
#include <cstdint>

#include "intrusive_list.h"
#include "snw_lang.h"
#include "message_stream.h"
#include "slot_allocator.h"

template<typename T>
class table {
public:
    table(size_t capacity)
        : nonce_(0)
        , rows_(new row[capacity])
        , row_allocator_(capacity)
    {
    }

    template<typename... Args>
    int32_t add(Args&&... args) {
        uint64_t row_id;
        if (!row_allocator_.allocate(&row_id)) {
            return -1;
        }

        row& row = rows_[row_id];
        row.nonce = ++nonce_;
        new(&rows_[row_id].data) T(std::forward<Args>(args)...);
        return static_cast<int32_t>(row_id);
    }

    void remove(int32_t row_id) {
        row& row = rows_[row_id];
        row.nonce = 0;
        reinterpret_cast<T*>(row.data)->~T();
        row_allocator_.deallocate(row_id);
        ++nonce_;
    }

    template<typename U>
    void update(int32_t row_id, U&& value) {
        row& row = rows_[row_id];
        *reinterpret_cast<T*>(row.data) = std::forward<U>(value);
    }

    const T& find(int32_t row_id) const {
        const row& row = rows_[row_id];
        return *reinterpret_cast<const T*>(row.data);
    }

    template<typename F>
    void scan(F&& f) const {
        uint64_t nonce = nonce_;
        row_allocator_.scan([&](uint64_t row_id) {
            const row& row = rows_[row_id];
            if ((0 < row.nonce) && (row.nonce <= nonce)) {
                f(static_cast<int32_t>(row_id), *reinterpret_cast<const T*>(row.data));
            }
        });
    }

private:
    struct row {
        uint64_t                    nonce;
        alignas(alignof(T)) uint8_t data[sizeof(T)];
    };

    uint64_t               nonce_;
    std::unique_ptr<row[]> rows_;
    snw::slot_allocator    row_allocator_;
};

int main(int argc, char** argv) {
    table<int> tbl(1 << 20);

    int32_t rid1 = tbl.add(3);
    int32_t rid2 = tbl.add(4);

    std::cout << "rid1: " << rid1 << std::endl;
    std::cout << "rid2: " << rid2 << std::endl;

    tbl.scan([&](int32_t rid, int value) {
        std::cout << "rid: " << rid << std::endl;
        if (rid == rid1) {
            tbl.remove(rid2);
        }
    });

    return 0;
}
