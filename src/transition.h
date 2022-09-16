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

  template<uint8_t N>
  bool _entry() { return false; }
  void _exit() { }
  template<uint8_t N>
  EmptyState* _doit() { return 0; }
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
template<
  uint8_t Trigger,
  typename To,
  typename To2,
  typename From,
  typename CreationPolicy,
  typename Guard,
  typename Action,
  bool IsExitingTransition,
  bool IsEnteringTransition>
struct TransitionBase {
  enum { N = Trigger };
  enum { E = IsEnteringTransition };
  enum { X = IsExitingTransition };
  typedef To ToType;
  typedef From FromType;
  typedef typename CreationPolicy::ObjectType StateType;

  DispatchResult<StateType> dispatch(StateType* activeState) {
    typedef typename To::CreatorType ToFactory;
    typedef typename To2::CreatorType To2Factory;
    typedef typename From::CreatorType FromFactory;
    From* fromState = FromFactory::create();

    // Initial transition
    if (!is_same<EmptyState<StateType>, To>().value && is_same<EmptyState<StateType>, From>().value) {
      Action().perform(activeState);
      To* toState = ToFactory::create();
      bool cosumedBySubstate = toState->template _entry<N>();
      if (!cosumedBySubstate) {
        toState->template _doit<N>();
      }

      // Delete not needed. "activeState" and "fromState" are null (the initial state)

      return DispatchResult<StateType>(true, toState);
    }

    // End transition; to any state <- from AnyState
    if (is_same<From, AnyState<StateType>>().value) {

      // Delete fromState not needed; it is "null".

      if (Guard().check(activeState)) {
        // TODO: "exit" of AnyState is called, not from the activeState object. Polymorphism is required.
        static_cast<StateType*>(activeState)->_exit();
        Action().perform(activeState);

        // TODO: AnyState::destroy is called.
        typedef typename CreationPolicy::CreatorType Creator;
        typedef typename CreationPolicy::ObjectType Object;
        Creator::destroy(static_cast<Object*>(activeState));
        To* toState = ToFactory::create();
        return DispatchResult<StateType>(true, toState);
      }
      return DispatchResult<StateType>(false, activeState);
    }

    // The transition is valid if the "fromState" is also the activeState state from the state machine.
    if (activeState == 0 || !fromState->equals(*activeState)) {

      // Entering substate transition
      if (E && activeState == 0) {
        To* toState = ToFactory::create();
        bool cosumedBySubstate = toState->template _entry<N>();
        if (!cosumedBySubstate) {
          toState->template _doit<N>();
        }

        // Delete not needed. "activeState" and "fromState" are null (the initial state)

        return DispatchResult<StateType>(true, toState);
      }

      FromFactory::destroy(fromState);
      return DispatchResult<StateType>(false, activeState);
    }

    Action().perform(activeState);

    // Choice transition
    if (!is_same<To2, EmptyState<typename CreationPolicy::ObjectType>>().value) {

      if (Guard().check(activeState)) {
        To* toState = ToFactory::create();

        // Self transition
        if (!fromState->equals(*toState)) {
          static_cast<From*>(activeState)->_exit();
          FromFactory::destroy(static_cast<From*>(activeState));
        }
        FromFactory::destroy(fromState);
        bool cosumedBySubstate = toState->template _entry<N>();
        if (!cosumedBySubstate) {
          toState->template _doit<N>();
        }
        return DispatchResult<StateType>(true, toState, X);
      }
      else {
        To2* toState = To2Factory::create();

        // Self transition
        if (fromState->equals(*toState)) {
          toState->template _doit<N>();
          return DispatchResult<StateType>(true, toState, X);
        }

        FromFactory::destroy(static_cast<From*>(activeState));
        static_cast<From*>(activeState)->_exit();
        FromFactory::destroy(fromState);
        bool cosumedBySubstate = toState->template _entry<N>();
        if (!cosumedBySubstate) {
          toState->template _doit<N>();
        }
        return DispatchResult<StateType>(true, toState, X);
      }
    }

    FromFactory::destroy(fromState);

    if (!Guard().check(activeState)) {
      return DispatchResult<StateType>(false, activeState);
    }

    

    // Self transition
    if (is_same<To, From>().value) {
      auto state = static_cast<To*>(activeState)->template _doit<N>();
      return DispatchResult<StateType>(true, state != 0 ? state : activeState);
    }

    static_cast<From*>(activeState)->_exit();

    if (X) {
      FromFactory::destroy(static_cast<From*>(activeState));
      To* toState = ToFactory::create();
      return DispatchResult<StateType>(true, toState, X);
    }

    To* toState = ToFactory::create();
    bool cosumedBySubstate = toState->template _entry<N>();
    if (!cosumedBySubstate) {
      toState->template _doit<N>();
    }
    FromFactory::destroy(static_cast<From*>(activeState));
    return DispatchResult<StateType>(true, toState, X);
  }
};
}

template<typename To, typename CreationPolicy, typename Action>
struct InitialTransition : impl::TransitionBase<0, To, EmptyState<typename CreationPolicy::ObjectType>, EmptyState<typename CreationPolicy::ObjectType>, CreationPolicy, OkGuard, Action, false, true> {
};

template<uint8_t Trigger, typename From, typename CreationPolicy, typename Guard, typename Action>
struct FinalTransition : impl::TransitionBase<Trigger, EmptyState<typename CreationPolicy::ObjectType>, EmptyState<typename CreationPolicy::ObjectType>, From, CreationPolicy, Guard, Action, false, false> {
  FinalTransition() {
    // Make sure the user defines a guard for the final transition. This is not UML compliant.
    CompileTimeError < !is_same<Guard, OkGuard>().value > ();
  }
};

template<typename CreationPolicy, typename Guard, typename Action>
struct EndTransition : impl::TransitionBase<0, EmptyState<typename CreationPolicy::ObjectType>, EmptyState<typename CreationPolicy::ObjectType>, AnyState<typename CreationPolicy::ObjectType>, CreationPolicy, Guard, Action, false, false> {
  EndTransition() {
    // Final transition without guard does not make sense; the state machine would immediately go to the final state.
    CompileTimeError < !is_same<Guard, OkGuard>().value > ();
  }
};

template<typename CreationPolicy>
using NullEndTransition = impl::TransitionBase<0, EmptyState<typename CreationPolicy::ObjectType>, EmptyState<typename CreationPolicy::ObjectType>, AnyState<typename CreationPolicy::ObjectType>, CreationPolicy, OkGuard, EmptyAction, false, false>;

template<uint8_t Trigger, typename Me, typename CreationPolicy, typename Guard, typename Action>
using SelfTransition = impl::TransitionBase<Trigger, Me, EmptyState<typename CreationPolicy::ObjectType>, Me, CreationPolicy, Guard, Action, false, false>;

template<uint8_t Trigger, typename Me, typename CreationPolicy>
using Declaration = impl::TransitionBase<Trigger, Me, EmptyState<typename CreationPolicy::ObjectType>, Me, CreationPolicy, OkGuard, EmptyAction, false, false>;

template<uint8_t Trigger, typename To, typename Me, typename CreationPolicy, typename Action = EmptyAction>
using ExitDeclaration = impl::TransitionBase<Trigger, To, EmptyState<typename CreationPolicy::ObjectType>, Me, CreationPolicy, OkGuard, Action, true, false>;

template<uint8_t Trigger, typename To, typename CreationPolicy, typename Action>
using EntryDeclaration = impl::TransitionBase<Trigger, To, EmptyState<typename CreationPolicy::ObjectType>, To, CreationPolicy, OkGuard, Action, false, true>;

template<uint8_t Trigger, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using ExitTransition = impl::TransitionBase<Trigger, To, EmptyState<typename CreationPolicy::ObjectType>, From, CreationPolicy, Guard, Action, true, false>;

template<uint8_t Trigger, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using Transition = impl::TransitionBase<Trigger, To, EmptyState<typename CreationPolicy::ObjectType>, From, CreationPolicy, Guard, Action, false, false>;

template<uint8_t Trigger, typename To_true, typename To_false, typename From, typename CreationPolicy, typename Guard, typename Action>
struct ChoiceTransition : impl::TransitionBase<Trigger, To_true, To_false, From, CreationPolicy, Guard, Action, false, false> {
  // Choice without guard does not make sense; the choice is either to go the the state To_true or To_false.
  ChoiceTransition() { CompileTimeError < !is_same<Guard, OkGuard>().value >(); }
};

}
