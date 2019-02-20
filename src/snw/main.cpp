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

void print_fn(process& proc) {
}

void add_fn(process& proc) {
}

int main(int argc, char** argv) {
    auto proc = make_process("(+ (+ 1 2) 3)");

    proc->env["eval"] = &eval_fn;
    proc->env["print"] = &print_fn;
    proc->env["+"] = &add_fn;

    eval_fn(*proc);

#ifdef SNW_OS_WINDOWS
    std::system("pause");
#endif
    return 0;
}
