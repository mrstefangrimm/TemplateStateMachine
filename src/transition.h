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
  DispatchResult(bool consumed, T* activeState, bool deferredEntry = false) {
    this->consumed = consumed;
    this->activeState = activeState;
    this->deferredEntry = deferredEntry;
  }
  bool consumed;
  bool deferredEntry;
  T* activeState;
};

template<typename T>
struct EmptyState : T {
  typedef EmptyState CreatorType;
  typedef EmptyState ObjectType;

  static EmptyState* create() {
    return 0;
  }
  static void destroy(EmptyState*) { }

  bool entry_() { return false; }
  void exit_() { }
  template<uint8_t N>
  EmptyState* doit_() { return 0; }
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
template<uint8_t Trigger, typename To, typename From, typename CreationPolicy, typename Guard, typename Action, bool IsExitingTransition>
struct TransitionBase {
  enum { N = Trigger };
  enum { X = IsExitingTransition };
  typedef To ToType;
  typedef From FromType;
  typedef typename CreationPolicy::ObjectType StateType;

  DispatchResult<StateType> dispatch(StateType* activeState) {
    typedef typename To::CreatorType ToFactory;
    typedef typename From::CreatorType FromFactory;
    To* toState = ToFactory::create();
    From* fromState = FromFactory::create();

    // Initial transition
    if (!is_same<EmptyState<StateType>, To>().value && is_same<EmptyState<StateType>, From>().value) {
      Action().perform(activeState);
      toState->entry_();
      toState->template doit_<Trigger>();

      // Delete not needed. "activeState" and "fromState" are null (the initial state)

      return DispatchResult<StateType>(true, toState);
    }

    // End transition; to any state <- from AnyState
    if (is_same<From, AnyState<StateType>>().value) {

      // Delete toState and fromState not needed; both are "null".

      if (Guard().check(activeState)) {
        // TODO: "exit" of AnyState is called, not from the activeState object. Polymorphism is required.
        static_cast<StateType*>(activeState)->exit_();
        Action().perform(activeState);

        // TODO: AnyState::destroy is called.
        typedef typename CreationPolicy::CreatorType Creator;
        typedef typename CreationPolicy::ObjectType Object;
        Creator::destroy(static_cast<Object*>(activeState));
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

    Action().perform(activeState);

    if (!Guard().check(activeState)) {
      ToFactory::destroy(toState);
      return DispatchResult<StateType>(false, activeState);
    }

    // Self transition
    if (is_same<To, From>().value) {
      auto state = static_cast<To*>(activeState)->template doit_<Trigger>();
      ToFactory::destroy(toState);
      return DispatchResult<StateType>(true, state != 0 ? state : activeState);
    }

    static_cast<From*>(activeState)->exit_();

    if (X) {
      FromFactory::destroy(static_cast<From*>(activeState));
      return DispatchResult<StateType>(true, toState, X);
    }
    bool cosumedBySubstate = toState->entry_();
    // TODO: Does not work in subStates.begin when the same trigger (e.g timeout) is defined for the substate (see washingmachine)
    if (!cosumedBySubstate) {
      toState->template doit_<Trigger>();
    }
    FromFactory::destroy(static_cast<From*>(activeState));
    return DispatchResult<StateType>(true, toState, X);
  }
};
}

template<typename To, typename CreationPolicy, typename Action>
struct InitialTransition : impl::TransitionBase<0, To, EmptyState<typename CreationPolicy::ObjectType>, CreationPolicy, OkGuard, Action, false> {
};

template<uint8_t Trigger, typename From, typename CreationPolicy, typename Guard, typename Action>
struct FinalTransition : impl::TransitionBase<Trigger, EmptyState<typename CreationPolicy::ObjectType>, From, CreationPolicy, Guard, Action, false> {
  FinalTransition() {
    // Make sure the user defines a guard for the final transition. This is not UML compliant.
    CompileTimeError < !is_same<Guard, OkGuard>().value > ();
  }
};

template<typename CreationPolicy, typename Guard, typename Action>
struct EndTransition : impl::TransitionBase<0, EmptyState<typename CreationPolicy::ObjectType>, AnyState<typename CreationPolicy::ObjectType>, CreationPolicy, Guard, Action, false> {
  EndTransition() {
    // Final transition without guard does not make sense; the state machine would immediately go to the final state.
    CompileTimeError < !is_same<Guard, OkGuard>().value > ();
  }
};

template<typename CreationPolicy>
using NullEndTransition = impl::TransitionBase<0, EmptyState<typename CreationPolicy::ObjectType>, AnyState<typename CreationPolicy::ObjectType>, CreationPolicy, OkGuard, EmptyAction, false>;

template<uint8_t Trigger, typename Me, typename CreationPolicy, typename Guard, typename Action>
using SelfTransition = impl::TransitionBase<Trigger, Me, Me, CreationPolicy, Guard, Action, false>;

template<uint8_t Trigger, typename Me, typename CreationPolicy>
using Declaration = impl::TransitionBase<Trigger, Me, Me, CreationPolicy, OkGuard, EmptyAction, false>;

template<uint8_t Trigger, typename To, typename Me, typename CreationPolicy>
using ExitDeclaration = impl::TransitionBase<Trigger, To, Me, CreationPolicy, OkGuard, EmptyAction, true>;

template<uint8_t Trigger, typename To, typename Me, typename CreationPolicy, typename Guard>
using ExitDeclaration2 = impl::TransitionBase<Trigger, To, Me, CreationPolicy, Guard, EmptyAction, true>;

template<uint8_t Trigger, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using ExitTransition = impl::TransitionBase<Trigger, To, From, CreationPolicy, Guard, Action, true>;

template<uint8_t Trigger, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using Transition = impl::TransitionBase<Trigger, To, From, CreationPolicy, Guard, Action, false>;

}
