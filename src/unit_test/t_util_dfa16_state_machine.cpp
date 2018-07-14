#include "catch.hpp"
#include "dfa16_state_machine.h"

using dfa16_state_machine = snw::dfa16_state_machine;
using dfa16_transition_table = snw::dfa16_transition_table;
using dfa16_state = dfa16_transition_table::state;
using dfa16_event = dfa16_transition_table::event;

static const dfa16_transition_table grammar_12345 = []() {
    dfa16_transition_table transitions;
    for(uint8_t s = 0; s < 5; ++s) {
        transitions.add_transition(('1'+s), s, (s+1));
    }

    return transitions;
}();

TEST_CASE("dfa16_transition_table") {
    // TODO
}

TEST_CASE("dfa16_state_machine") {
    SECTION("initial state is initially the current state") {
        dfa16_state_machine stm(&grammar_12345);

        CHECK(stm.current_state() == 0);
    }

    SECTION("partial match") {
        dfa16_state_machine stm(&grammar_12345);

        const dfa16_event ev[3] = { '1', '2', '3' };
        CHECK(stm.run(ev, 3) == 3);
    }

    SECTION("full match") {
        dfa16_state_machine stm(&grammar_12345);

        const dfa16_event ev[5] = { '1', '2', '3', '4', '5' };
        CHECK(stm.run(ev, 5) == 5);
    }

    SECTION("no match") {
        dfa16_state_machine stm(&grammar_12345);

        const dfa16_event ev[5] = { 'a', 'b', 'c', 'd', 'e' };
        CHECK(stm.run(ev, 5) == 0xF);
    }

    SECTION("observe transitions") {
        dfa16_state_machine stm(&grammar_12345);

        int i = 0;
        const dfa16_event ev[3] = { '1', '2', '3' };
        stm.run(ev, 3, [&](const dfa16_event* ep, dfa16_state s0, dfa16_state s1) {
            switch(i++) {
            case 0:
                CHECK(ep == &ev[0]);
                CHECK(s0 == 0);
                CHECK(s1 == 1);
                break;

            case 1:
                CHECK(ep == &ev[1]);
                CHECK(s0 == 1);
                CHECK(s1 == 2);
                break;

            case 2:
                CHECK(ep == &ev[2]);
                CHECK(s0 == 2);
                CHECK(s1 == 3);
                break;

            default:
                CHECK(false);
            }
        });
    }
}
