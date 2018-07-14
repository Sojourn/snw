#pragma once

#include <cstring>
#include "types.h"

namespace snw {

class dfa16_transition_table {
public:
    using state = uint8_t;
    using event = char;

    dfa16_transition_table(state default_state = 0x0F);

    void add_transition(event e, state s0, state s1);
    state transition(event e, state s) const;

private:
    uint8_t table_[256][16];
};

// deterministic finite automaton
class dfa16_state_machine {
public:
    using state = dfa16_transition_table::state;
    using event = dfa16_transition_table::event;

    dfa16_state_machine(const dfa16_transition_table* transitions, state initial_state);

    state run(const event* ev, size_t ev_cnt);
    state run(event e);

    template <typename TransitionObserver>
    state run(const event* ev, size_t ev_cnt, TransitionObserver&& transition_observer);

    template <typename TransitionObserver>
    state run(event e, TransitionObserver&& transition_observer);

    state current_state() const;
    void reset(state initial_state);

private:
    const dfa16_transition_table* transitions_;
    state state_;
};

inline dfa16_transition_table::state dfa16_transition_table::transition(event e, state s) const
{
    return table_[static_cast<uint8_t>(e)][s];
}

inline dfa16_state_machine::state dfa16_state_machine::current_state() const
{
    return state_;
}

inline void dfa16_state_machine::reset(state initial_state)
{
    state_ = initial_state;
}

inline dfa16_state_machine::state dfa16_state_machine::run(const event* ev, size_t ev_cnt)
{
    state s = current_state();
    for(size_t i = 0; i < ev_cnt; ++i) {
        s = run(ev[i]);
    }
    return s;
}

inline dfa16_state_machine::state dfa16_state_machine::run(event e)
{
    state s0 = state_;
    state s1 = transitions_->transition(e, s0);
    return (state_ = s1);
}

template <typename TransitionObserver>
inline dfa16_state_machine::state dfa16_state_machine::run(const event* ev, size_t ev_cnt, TransitionObserver&& transition_observer)
{
    state s = current_state();
    for(size_t i = 0; i < ev_cnt; ++i) {
        s = run(ev[i], [&](event, state s0, state s1) {
            transition_observer(&ev[i], s0, s1);
        });
    }
    return s;
}

template <typename TransitionObserver>
inline dfa16_state_machine::state dfa16_state_machine::run(event e, TransitionObserver&& transition_observer)
{
    state s0 = state_;
    state s1 = transitions_->transition(e, s0);
    if(s0 != s1) {
        transition_observer(e, s0, s1);
    }
    return (state_ = s1);
}

}