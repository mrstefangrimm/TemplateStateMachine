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
  bool _entry() {
    return false;
  }
  template<uint8_t N>
  EmptyState* _doit() {
    return 0;
  }
};

/* Provides Action interface and does nothing. */
struct EmptyAction {
  template<typename T>
  void perform(T*) { }
};

/* Provides Guard interface and returns true. */
struct OkGuard {
  template<typename T>
  bool eval(T*) {
    return true;
  }
};

template<typename CreationPolicy>
struct NullTransition {
  typedef typename CreationPolicy::ObjectType StateType;
  typedef EmptyState<StateType> ToType;
  typedef EmptyState<StateType> FromType;
  typedef NullType CreationPolicyType;
  enum { N = -1 };
  enum { E = false };
  enum { X = false };

  DispatchResult<StateType> dispatch(StateType* activeState) {
    return DispatchResult< StateType>(false, 0, false);
  }
};

namespace impl {

template <
  uint8_t Trigger,
  typename To,
  typename From,
  typename CreationPolicy,
  typename Guard,
  typename Action,
  bool IsEnteringTransition,
  bool IsExitingTransition >
struct TransitionBase {
  enum { N = Trigger };
  enum { E = IsEnteringTransition };
  enum { X = IsExitingTransition };
  typedef CreationPolicy CreationPolicyType;
  typedef To ToType;
  typedef From FromType;
  typedef typename CreationPolicy::ObjectType StateType;

  DispatchResult<StateType> dispatch(StateType* activeState) {
    typedef typename From::CreatorType FromFactory;
    From* fromState = FromFactory::create();

    // Entering substate transition
    if (E) {
      typedef typename To::CreatorType ToFactory;
      To* toState = ToFactory::create();
      bool cosumedBySubstate = toState->template _entry<N>();
      if (!cosumedBySubstate) {
        toState->template _doit<N>();
      }

      FromFactory::destroy(fromState);
      return DispatchResult<StateType>(true, toState);
    }

    // The transition is valid if the "fromState" is also the activeState state from the state machine.
    if (activeState == 0 || !fromState->equals(*activeState)) {
      FromFactory::destroy(fromState);
      return DispatchResult<StateType>(false, activeState);
    }

    Action().perform(activeState);

    FromFactory::destroy(fromState);

    if (!Guard().eval(activeState)) {
      return DispatchResult<StateType>(false, activeState);
    }

    // Self transition
    if (is_same<To, From>().value) {
      auto state = static_cast<To*>(activeState)->template _doit<N>();
      return DispatchResult<StateType>(true, state != 0 ? state : activeState);
    }

    static_cast<From*>(activeState)->template _exit<N>();

    if (X) {
      FromFactory::destroy(static_cast<From*>(activeState));
      typedef typename To::CreatorType ToFactory;
      To* toState = ToFactory::create();
      return DispatchResult<StateType>(true, toState, X);
    }

    typedef typename To::CreatorType ToFactory;
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

// TODO: remove
template<uint8_t Trigger, typename From, typename CreationPolicy, typename Guard, typename Action>
struct FinalTransition : impl::TransitionBase<Trigger, EmptyState<typename CreationPolicy::ObjectType>, From, CreationPolicy, Guard, Action, false, false> {
  FinalTransition() {
    // Make sure the user defines a guard for the final transition. This is not UML compliant.
    CompileTimeError < !is_same<Guard, OkGuard>().value > ();
  }
};

template<uint8_t Trigger, typename Me, typename CreationPolicy, typename Guard, typename Action>
using SelfTransition = impl::TransitionBase<Trigger, Me, Me, CreationPolicy, Guard, Action, false, false>;

template<uint8_t Trigger, typename Me, typename CreationPolicy>
using Declaration = impl::TransitionBase<Trigger, Me, Me, CreationPolicy, OkGuard, EmptyAction, false, false>;

template<uint8_t Trigger, typename To, typename Me, typename CreationPolicy, typename Guard>
using ExitDeclaration = impl::TransitionBase<Trigger, To, Me, CreationPolicy, Guard, EmptyAction, false, true>;

template<uint8_t Trigger, typename To, typename CreationPolicy, typename Action>
using EntryDeclaration = impl::TransitionBase<Trigger, To, To, CreationPolicy, OkGuard, Action, true, false>;

template<uint8_t Trigger, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using ExitTransition = impl::TransitionBase<Trigger, To, From, CreationPolicy, Guard, Action, false, true>;

template<uint8_t Trigger, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using Transition = impl::TransitionBase<Trigger, To, From, CreationPolicy, Guard, Action, false, false>;

}
