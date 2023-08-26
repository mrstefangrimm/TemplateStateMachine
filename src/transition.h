#pragma once
/*
  Copyright 2022-2023 Stefan Grimm

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
#include "lokilight.h"

namespace tsmlib {

using namespace LokiLight;

template<class T>
struct DispatchResult {
  static DispatchResult null;

  DispatchResult(bool consumed, T* activeState) {
    this->consumed = consumed;
    this->activeState = activeState;
  }

  bool consumed;
  T* activeState;
};
template<class T> DispatchResult<T> DispatchResult<T>::null(false, nullptr);

template<class T>
struct EmptyState : T {
  using CreatorType = EmptyState<T>;
  using ObjectType = EmptyState<T>;

  static EmptyState* create() {
    return nullptr;
  }

  static void destroy(EmptyState*) {}

  template<class Event>
  void _entry(const Event&) {}

  template<class Event>
  bool _doit(const Event&) {
    return false;
  }
};

//Provides Action interface and does nothing.
struct NoAction {
  template<class StateType, class EventType>
  void perform(StateType&, const EventType&) {}
  void perform() {}
};

//Provides Guard interface and returns true.
struct NoGuard {
  template<class StateType, class EventType>
  bool eval(const StateType& state, const EventType&) {
    return true;
  }
};

namespace impl {

template<
  class Event,
  typename To,
  typename From,
  typename CreationPolicy,
  typename Guard,
  typename Action,
  bool IsEnteringTransition,
  bool IsExitingTransition,
  bool IsReenteringTransition,
  bool IsExitDeclaration = false >
struct TransitionBase {
  enum { E = IsEnteringTransition };
  enum { X = IsExitingTransition };
  enum { R = IsReenteringTransition };
  enum { D = IsExitDeclaration };  // top-state exit declaration triggers the exit of the sub-states.

  using EventType = Event;
  using CreationPolicyType = CreationPolicy;
  using ToType = To;
  using FromType = From;
  using StateType = typename CreationPolicy::ObjectType;

  DispatchResult<StateType> dispatch(StateType* activeState, const Event& ev) {
    using FromFactory = typename From::CreatorType;

    // Ignore the transition if the active state is null.
    if (activeState == nullptr) {
      return DispatchResult<StateType>(false, activeState);
    }

    // Entering substate transition
    if (E) {
      using ToFactory = typename To::CreatorType;
      To* toState = ToFactory::create();
      toState->template _entry<EventType>(ev);
      if (is_base_of< BasicState< To, StateType >, To >::value) {
        toState->_doit(ev);
      }

      return DispatchResult<StateType>(true, toState);
    }

    FromType* fromState = static_cast<FromType*>(activeState);
    Action().perform(*fromState, ev);

    if (!Guard().eval(*fromState, ev)) {
      return DispatchResult<StateType>(false, activeState);
    }

    // Self transition
    if (is_same<To, From>().value || D) {

      // Exit and enter when it is a reentering transition
      if (R) {
        static_cast<From*>(activeState)->template _exit<EventType>(ev);
        static_cast<From*>(activeState)->template _entry<EventType>(ev);
      }

      const bool consumed = static_cast<From*>(activeState)->template _doit<EventType>(ev);

      // Exit declaration (on the top level)
      if (D) {
        if (consumed) {
          return DispatchResult<StateType>(true, activeState);
        }

        static_cast<From*>(activeState)->template _exit<EventType>(ev);

        using ToFactory = typename To::CreatorType;
        To* toState = ToFactory::create();
        toState->template _entry<EventType>(ev);
        if (is_base_of< BasicState< To, StateType >, To >::value) {
          toState->template _doit<EventType>(ev);
        }
        FromFactory::destroy(static_cast<From*>(activeState));
        return DispatchResult<StateType>(true, toState);
      }
      return DispatchResult<StateType>(consumed, activeState);
    }

    if (X) {
      return DispatchResult<StateType>(false, activeState);
    }

    static_cast<From*>(activeState)->template _exit<EventType>(ev);

    using ToFactory = typename To::CreatorType;
    To* toState = ToFactory::create();
    toState->template _entry<EventType>(ev);
    if (is_base_of< BasicState< To, StateType >, To >::value) {
      toState->_doit(ev);
    }
    FromFactory::destroy(static_cast<From*>(activeState));
    return DispatchResult<StateType>(true, toState);
  }

};
}

template<class Event, typename Me, typename CreationPolicy, typename Guard, typename Action, bool Reenter>
using SelfTransition = impl::TransitionBase<Event, Me, Me, CreationPolicy, Guard, Action, false, false, Reenter>;

template<class Event, typename Me, typename CreationPolicy>
using Declaration = impl::TransitionBase<Event, Me, Me, CreationPolicy, NoGuard, NoAction, false, false, false>;

template<class Event, typename To, typename Me, typename CreationPolicy>
using ExitDeclaration = impl::TransitionBase<Event, To, Me, CreationPolicy, NoGuard, NoAction, false, false, false, true>;

template<class Event, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using ExitTransition = impl::TransitionBase<Event, To, From, CreationPolicy, Guard, Action, false, true, false>;

template<class Event, typename To, typename From, typename CreationPolicy, typename Guard, typename Action>
using Transition = impl::TransitionBase<Event, To, From, CreationPolicy, Guard, Action, false, false, false>;

}
