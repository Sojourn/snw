#include "object_handle.h"
#include "object_heap.h"

void snw::object_handle::attach() {
    detach();
    heap_->register_root(*this);
}

void snw::object_handle::detach() {
    ref_.unlink();
}
