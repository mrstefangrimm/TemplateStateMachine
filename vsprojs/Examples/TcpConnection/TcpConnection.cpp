// Let's release a TCP connection!
// https://github.com/boost-ext/sml
//
// Programmed with the TSM
//
#define IAMWORKSTATION 1

#include <cstdio>
#include <cassert>

#include <iostream>
#include "../../../src/tsm.h"

using namespace tsmlib;

using StateType = State<MemoryAddressComparator, true>;
using StateTypeCreationPolicyType = SingletonCreator<StateType>;


// events
struct close;
struct ack {
  bool valid{};
};
struct fin {
  int id{};
  int valid{};
};
struct close {};
struct timeout {};

// guards

struct is_valid {
  template<class StateType, class EventType>
  bool eval(StateType*, const EventType& ev) {
    return ev.valid;
  }
};

// actions

struct send_fin {
  template<class StateType, class EventType>
  void perform(StateType*, const EventType&) {
    fin msg{ 0 };
    std::printf("send: %d\n", msg.id);
  }
};

struct send_ack {
  template<class StateType, class EventType>
  void perform(StateType*, const EventType& ev) {
    std::printf("send: %d\n", ev.id);
  }
};

// states

struct established : public BasicState<established, StateType>, public SingletonCreator<established> {
  void entry() {}
  void exit() {}
  template<class event> void doit() {}
};

struct fin_wait_1 : public BasicState<fin_wait_1, StateType>, public SingletonCreator<fin_wait_1> {
  void entry() {}
  void exit() {}
  template<class Event> void doit() {}
};

struct fin_wait_2 : public BasicState<fin_wait_2, StateType>, public SingletonCreator<fin_wait_2> {
  void entry() {}
  void exit() {}
  template<class Event> void doit() {}
};

struct timed_wait : public BasicState<timed_wait, StateType>, public SingletonCreator<timed_wait> {
  void entry() {}
  void exit() {}
  template<class Event> void doit() {}
};

using ToWait1FromEstablished = Transition<close, fin_wait_1, established, StateTypeCreationPolicyType, NoGuard, send_fin>;
using ToWait1FromWait1 = Transition<ack, fin_wait_2, fin_wait_1, StateTypeCreationPolicyType, is_valid, NoAction>;
using ToTimedFromWait2 = Transition<fin, timed_wait, fin_wait_2, StateTypeCreationPolicyType, is_valid, send_ack>;
using ToFinalFromTimed = FinalTransitionExplicit<timeout, timed_wait, StateTypeCreationPolicyType, NoGuard, NoAction>;

using Transitions =
  Typelist<ToWait1FromEstablished,
  Typelist<ToWait1FromWait1,
  Typelist<ToTimedFromWait2,
  Typelist<ToFinalFromTimed,
  NullType>>>>;

using InitTransition = InitialTransition<established, StateTypeCreationPolicyType, NoAction>;
using Sm = Statemachine<Transitions, InitTransition>;

int main()
{
  Sm sm;
  auto result = sm.begin();

  //assert(sm.is("established"_s));

  result = sm.dispatch<close>(close{}); // complexity O(1)
  //assert(sm.is("fin wait 1"_s));

  result = sm.dispatch(ack{ true });
  //assert(sm.is("fin wait 2"_s));

  result = sm.dispatch(fin{ 42, true });
  //assert(sm.is("timed wait"_s));

  result = sm.dispatch<timeout>(timeout{});
  //assert(sm.is(X));  // terminated
}
