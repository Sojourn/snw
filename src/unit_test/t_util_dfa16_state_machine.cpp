#include "catch.hpp"
#include "dfa16_state_machine.h"

using dfa16_state_machine = snw::dfa16_state_machine;
using dfa16_transition_table = snw::dfa16_transition_table;

static dfa16_transition_table transition_table = []() {
    return dfa16_transition_table();
}();

TEST_CASE("dfa16_state_machine") {
    CHECK(transition_table.transition('A', 0) == 0x0F);
}
