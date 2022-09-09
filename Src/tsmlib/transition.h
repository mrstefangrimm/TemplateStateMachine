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

template<typename T>
struct DispatchResult {
  DispatchResult(bool consumed, T* activeState) {
    this->consumed = consumed;
    this->activeState = activeState;
  }
  bool consumed;
  T* activeState;
};

template<typename T>
struct EmptyState : T {
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

namespace impl {
  template<uint8_t Trigger, typename StateType, typename To, typename From, typename Guard, typename Action, bool IsExitingTransition>
  struct TransitionBase {
    enum { N = Trigger };
    enum { E = IsExitingTransition };
    typedef From FromType;

    DispatchResult<StateType> dispatch(StateType* activeState) {
      typedef typename To::CreatorType ToFactory;
      typedef typename From::CreatorType FromFactory;
      To* toState = ToFactory::create();
      From* fromState = FromFactory::create();

      // Initial transition
      if (!is_same<EmptyState<StateType>, To>().value && is_same<EmptyState<StateType>, From>().value) {
        Action().perform(static_cast<From*>(activeState));
        toState->entry();
        toState->template doit<Trigger>();

        // Delete not needed. "activeState" and "fromState" are null (the initial state)

        return DispatchResult<StateType>(true, toState);
      }

      // Final transition Any <- Any
      if (is_same<AnyState<StateType>, From>().value) {

        // Delete toState and fromState not needed; both are "null".

        if (Guard().check(static_cast<From*>(activeState))) {
          // TODO: "exit" of AnyState is called, not from the activeState object. Polymorphism is required.
          static_cast<From*>(activeState)->exit();
          Action().perform(static_cast<From*>(activeState));
          // TODO: AnyState::Delete is called
          FromFactory::destroy(static_cast<From*>(activeState));
          return DispatchResult<StateType>(true, toState);
        }
        return DispatchResult<StateType>(false, activeState);
      }

      // The transition is valid if the "fromState" is also the activeState state from the state machine.
      if (activeState == 0 || !fromState->equals(*activeState)) {
        ToFactory::destroy(toState);
        FromFactory::destroy(fromState);
        return DispatchResult<StateType>(false, activeState);
      }
      FromFactory::destroy(fromState);

      if (!Guard().check(static_cast<From*>(activeState))) {
        ToFactory::destroy(toState);
        return DispatchResult<StateType>(false, activeState);
      }
      // Self transition
      if (is_same<To, From>().value) {
        Action().perform(static_cast<From*>(activeState));
        static_cast<To*>(activeState)->template doit<Trigger>();
        ToFactory::destroy(toState);
        return DispatchResult<StateType>(true, activeState);
      }

      static_cast<From*>(activeState)->exit();

      Action().perform(static_cast<From*>(activeState));
      toState->entry();
      toState->template doit<Trigger>();
      FromFactory::destroy(static_cast<From*>(activeState));
      return DispatchResult<StateType>(true, toState);
    }
  };
}

template<typename StateType, typename To, typename Action>
struct InitialTransition : impl::TransitionBase<0, StateType, To, EmptyState<StateType>, OkGuard, Action, false> {
};

template<uint8_t Trigger, typename StateType, typename From, typename Guard, typename Action>
struct FinalTransition : impl::TransitionBase<Trigger, StateType, EmptyState<StateType>, From, Guard, Action, false> {
  FinalTransition() {
    // Make sure the user defines a guard for the final transition. This is not UML compliant.
    CompileTimeError < !is_same<Guard, OkGuard>().value > ();
  }
};

template<typename StateType, typename Guard, typename Action>
struct EndTransition : impl::TransitionBase<0, StateType, EmptyState<StateType>, AnyState<StateType>, Guard, Action, false> {
  EndTransition() {
    // Final transition without guard does not make sense; the state machine would immediately go to the final state.
    CompileTimeError < !is_same<Guard, OkGuard>().value >();
  }
};

template<typename StateType>
using NullEndTransition = impl::TransitionBase<0, StateType, EmptyState<StateType>, AnyState<StateType>, OkGuard, EmptyAction, false>;

template<uint8_t Trigger, typename StateType, typename Me, typename Guard, typename Action>
using SelfTransition = impl::TransitionBase<Trigger, StateType, Me, Me, Guard, Action, false>;

template<uint8_t Trigger, typename StateType, typename Me>
using Declaration = impl::TransitionBase<Trigger, StateType, Me, Me, OkGuard, EmptyAction, false>;

template<uint8_t Trigger, typename StateType, typename To, typename From, typename Guard, typename Action>
using ExitTransition = impl::TransitionBase<Trigger, StateType, To, From, Guard, Action, true>;

template<uint8_t Trigger, typename StateType, typename To, typename From, typename Guard, typename Action>
using Transition = impl::TransitionBase<Trigger, StateType, To, From, Guard, Action, false>;

}
