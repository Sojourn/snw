#include <cassert>
#include "dfa16_state_machine.h"

snw::dfa16_transition_table::dfa16_transition_table(state default_state)
    : table_()
{
    assert(default_state < 16);
    memset(table_, default_state, sizeof(table_));
}

void snw::dfa16_transition_table::add_transition(event e, state s0, state s1)
{
    assert((s0 < 16) && (s1 < 16));

    table_[static_cast<uint8_t>(e)][s0] = s1;
}

snw::dfa16_state_machine::dfa16_state_machine(const dfa16_transition_table* transitions, state initial_state)
    : transitions_(transitions)
    , state_(initial_state)
{
    assert(initial_state < 16);
}
