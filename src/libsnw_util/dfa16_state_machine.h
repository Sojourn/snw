#pragma once

#include "types.h"

namespace snw {

// deterministic finite automaton
class dfa16_state_machine {
public:
    using state = uint8_t;
    using event = char;

    class transition_table {
    public:
        transition_table(state default_state = 0x0F);

        void add_transition(event e, state s0, state s1);
        state transition(event e, state s) const;

    private:
        uint8_t table_[256][16];
    };

    dfa16_state_machine(const transition_table* transitions, state initial_state);

    state run(const event* ev, size_t ev_cnt);
    state run(event e);

    template <typename TransitionObserver>
    state run(const event* ev, size_t ev_cnt, TransitionObserver&& transition_observer);

    template <typename TransitionObserver>
    state run(event e, TransitionObserver&& transition_observer);

    state current_state() const;
    void reset();

private:
    const transition_table* transitions_;
    state s_;
};

}