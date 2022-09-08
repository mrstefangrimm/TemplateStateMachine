#pragma once
/*
  Copyright 2022 Stefan Grimm

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "state.h"
#include "templatemeta.h"

namespace tsmlib {

using namespace Loki;
using namespace std;

template<typename State>
struct DispatchResult {
  DispatchResult(bool consumed, State* state) {
    this->consumed = consumed;
    activeState = state;
  }
  bool consumed;
  State* activeState;
};

template<typename State>
struct EmptyState : State {
  typedef EmptyState CreatorType;

  static EmptyState* create() {
    return 0;
  }
  static void destroy(EmptyState*) { }

  void entry() { }
  void exit() { }
  template<uint8_t N>
  void doit() { }
};

/* Provides Action interface and does nothing. */
struct EmptyAction {
  template<typename T>
  void perform(T*) { }
};

/* Provides Guard interface and returns true. */
struct OkGuard {
  template<typename T>
  bool check(T*) {
    return true;
  }
};

template<uint8_t Trigger, typename State, typename To, typename From, typename Guard, typename Action, bool IsExitingTransition>
struct Transition {
  enum { N = Trigger };
  enum { E = IsExitingTransition };
  typedef From FromType;

  DispatchResult<State> dispatch(State* activeState) {
    typedef typename To::CreatorType ToFactory;
    typedef typename From::CreatorType FromFactory;
    To* toState = ToFactory::create();
    From* fromState = FromFactory::create();

    // Initial transition
    if (!is_same<EmptyState<State>, To>().value && is_same<EmptyState<State>, From>().value) {
      Action().perform(static_cast<From*>(activeState));
      toState->entry();
      toState->template doit<Trigger>();

      // Delete not needed. "activeState" and "fromState" are null (the initial state)

      return DispatchResult<State>(true, toState);
    }

    // Final transition Any <- Any
    if (is_same<AnyState<State>, From>().value) {

      // Delete toState and fromState not needed; both are "null".

      if (Guard().check(static_cast<From*>(activeState))) {
        // TODO: "exit" of AnyState is called, not from the activeState object. Polymorphism is required.
        static_cast<From*>(activeState)->exit();
        Action().perform(static_cast<From*>(activeState));
        // TODO: AnyState::Delete is called
        FromFactory::destroy(static_cast<From*>(activeState));
        return DispatchResult<State>(true, toState);
      }
      return DispatchResult<State>(false, activeState);
    }

    // The transition is valid if the "fromState" is also the activeState state from the state machine.
    if (!fromState->equals(*activeState)) {
      ToFactory::destroy(toState);
      FromFactory::destroy(fromState);
      return DispatchResult<State>(false, activeState);
    }
    FromFactory::destroy(fromState);

    if (!Guard().check(static_cast<From*>(activeState))) {
      ToFactory::destroy(toState);
      return DispatchResult<State>(false, activeState);
    }
    // Self transition
    if (is_same<To, From>().value) {
      Action().perform(static_cast<From*>(activeState));
      static_cast<To*>(activeState)->template doit<Trigger>();
      ToFactory::destroy(toState);
      return DispatchResult<State>(true, activeState);
    }

    static_cast<From*>(activeState)->exit();

    Action().perform(static_cast<From*>(activeState));
    toState->entry();
    toState->template doit<Trigger>();
    FromFactory::destroy(static_cast<From*>(activeState));
    return DispatchResult<State>(true, toState);
  }
};

template<typename State, typename To, typename Action>
struct InitialTransition : Transition<0, State, To, EmptyState<State>, OkGuard, Action, false> {
};

template<typename STATE, typename Guard, typename ACTION>
struct FinalTransition : Transition<0, STATE, EmptyState<STATE>, AnyState<STATE>, Guard, ACTION, false> {
  FinalTransition() {
    // Final transition without guard does not make sense; the state machine would immediately go to the final state.
    CompileTimeError < !is_same<Guard, OkGuard>().value > ();
  }
};

template<typename State>
using  NullFinalTransition = Transition<0, State, EmptyState<State>, AnyState<State>, OkGuard, EmptyAction, false>;

}
