#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <array>
#include <bitset>
#include <memory>
#include <atomic>
#include <cstring>
#include <cstdint>
#include <cstddef>

#include "snw_util.h"
#include "snw_event.h"

#include "object.h"
#include "object_heap.h"
#include "object_stack.h"
#include "object_transaction.h"
#include "parser.h"

using namespace snw;

struct process;
using function = void(*)(process&);
using environment = std::map<symbol, function>;

struct process {
    object_heap  heap;
    object_stack stack;
    environment  env;
};

std::unique_ptr<process> make_process(const char* program) {
    std::unique_ptr<process> proc(new process);
    auto& heap = proc->heap;
    auto& stack = proc->stack;

    stack.push(parser(heap).parse(program));
    return proc;
}

void eval_fn(process& proc) {
    if (proc.stack.empty()) {
        proc.stack.push(proc.heap.new_nil());
        return;
    }

    // ???
}

void add_fn(process& proc) {
}

// class list_builder {
// public:
//     list_builder(object_heap& heap)
//         : heap_(heap)
//     {
//     }

//     void push_back(object_ref ref) {
//         refs_.push_back(ref);
//     }

//     object_ref make_list() {
//         if (refs_.empty()) {
//             return heap_.new_nil();
//         }

//         auto first = refs_.data();
//         auto last = first + refs_.size();
//         return heap_.new_list(first, last);
//     }

// private:
//     object_heap&           heap_;
//     array<object_ref, 128> refs_;
// };

template<typename Handler>
void match(const object_heap& heap, object_ref ref, Handler&& handler) {
    switch (ref.type) {
    case object_type::nil:
        handler(heap);
        break;
    case object_type::integer:
        handler(heap, heap.deref_integer(ref));
        break;
    case object_type::symbol:
        handler(heap, heap.deref_symbol(ref));
        break;
    case object_type::string:
        handler(heap, heap.deref_string(ref));
        break;
    case object_type::bytes:
        handler(heap, heap.deref_bytes(ref));
        break;
    case object_type::cell:
        handler(heap, heap.deref_cell(ref));
        break;
    }
}

void print(std::ostream& out, const object_heap& heap, object_ref ref) {
    struct printer {
        std::ostream& out;

        void operator()(const object_heap& heap) {
            out << "()";
        }
        void operator()(const object_heap& heap, int64_t i) {
            out << i;
        }
        void operator()(const object_heap& heap, const symbol& s) {
            out << s;
        }
        void operator()(const object_heap& heap, const string_object& s) {
            out << '"';
            for (size_t i = 0; i < s.len; ++i) {
                switch (char c = s.str[i]) {
                case '\t':
                    out << "\\t";
                    break;
                case '\r':
                    out << "\\r";
                    break;
                case '\n':
                    out << "\\n";
                    break;
                case '"':
                    out << "\\\"";
                    break;
                default:
                    out << c;
                    break;
                }
            }
            out << '"';
        }
        void operator()(const object_heap& heap, const bytes_object& b) {
            // TODO
        }
        void operator()(const object_heap& heap, cell_object cell) {
            out << "(";
            while (true) {
                print(out, heap, cell.car);

                if (cell.cdr.type == object_type::nil) {
                    break;
                }
                else if (cell.cdr.type == object_type::cell) {
                    out << " ";
                    cell = heap.deref_cell(cell.cdr);
                }
                else {
                    break;
                }
            }
            out << ")";
        }
    };

    printer p{ out };
    match(heap, ref, p);
}

int main(int argc, char** argv) {
    object_heap heap;
    object_ref program = parser(heap).parse("(+ 1 2)\n(divide (+ 3 4) 7)\n(\"Hello\tWorld\")");
    print(std::cout, heap, program);

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
