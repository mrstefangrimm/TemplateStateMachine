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
#include "lokilight.h"
#include "transition.h"

namespace tsmlib {

template<class Transitions, class Initialtransition>
class Statemachine {
public:
  using StateType = typename Initialtransition::StateType;

  DispatchResult<StateType> begin() {
    const auto result = Initialtransition().dispatch();
    if (result.consumed) {
      activeState_ = result.activeState;
    }
    return result;
  }

  // TODO: private: friend class SubstatesHolderState<...; instead of using "_"
  template<class Event>
  DispatchResult<StateType> _begin() {

    // Transitions can have initial transitions (for a higher-level state to a sub-state).
    // The default initial transition is added to the front and is therefore executed when no other was found.
    const int size = Length<Transitions>::value;
    const auto result = Initializer< Transitions, Event, size - 1 >::init();
    if (result.consumed) {
      activeState_ = result.activeState;
    }
    return result;
  }

  /**
      End requires an exit-transition, otherwise exit is not called.
    */
  DispatchResult<StateType> end() {

    if (activeState_ == nullptr) return DispatchResult<StateType>::null;

    const int size = Length<Transitions>::value;
    auto result = Finalizer< Transitions, size - 1 >::end(activeState_);
    if (result.consumed) {
      activeState_ = 0;
    }
    return result;
  }

  template<class Event>
  DispatchResult<StateType> _end() {
    const int size = Length<Transitions>::value;
    const auto result = Finalizer< Transitions, size - 1 >::end(activeState_);
    if (result.consumed) {
      activeState_ = 0;
    }
    return result;
  }

  template<class Event>
  DispatchResult<StateType> dispatch() {
    return dispatch(Event{});
  }

  template<class Event>
  DispatchResult<StateType> dispatch(const Event& ev) {

    if (activeState_ == nullptr) return DispatchResult<StateType>::null;

    const int size = Length<Transitions>::value;
    auto result = TriggerExecutor< Event, size - 1 >::execute(activeState_, &ev);

    // Transition not found, active state is not changed
    if (!result.consumed) {
      return DispatchResult<StateType>(false, activeState_);
    }

    activeState_ = result.activeState;
    return DispatchResult<StateType>(true, activeState_);
  }

private:
  StateType* activeState_ = 0;

public:
  template<class Event, int Index>
  struct TriggerExecutor {

    static DispatchResult<StateType> execute(StateType* activeState, const Event* ev) {
      // Finds last element in the list that meets the conditions.
      using CurrentTransition = typename TypeAt<Transitions, Index>::Result;
      using FromType = typename CurrentTransition::FromType::ObjectType;
      using EventType = typename CurrentTransition::EventType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = is_same<EventType, Event>().value && hasSameFromState;
      if (conditionMet) {
        const EventType* currentTransitionEvent = reinterpret_cast<const EventType*>(ev);
        auto result = CurrentTransition().dispatch(activeState, *currentTransitionEvent);
        return result;
      }
      // Recursion
      return TriggerExecutor< Event, Index - 1 >::execute(activeState, ev);
    }

    static void entry(StateType* entryState) {
      // Finds last element in the list that meets the conditions.
      using CurrentTransition = typename TypeAt<Transitions, Index>::Result;
      using ToType = typename CurrentTransition::ToType::ObjectType;
      using CreationPolicy = typename CurrentTransition::CreationPolicyType;

      // TODO: Mustn't be a choice transition
      //CompileTimeError < is_same<ToType, EmptyState<typename CreationPolicy::ObjectType>>().value >();

      bool hasSameFromState = entryState->template typeOf<ToType>();

      bool conditionMet = is_same<typename CurrentTransition::EventType, Event>().value&& hasSameFromState;
      if (conditionMet) {
        ToType* state = static_cast<ToType*>(entryState);
        state->template _entry<Event>();
        NullType nu;
        state->_doit(nu);
        return;
      }
      // Recursion
      TriggerExecutor< Event, Index - 1 >::entry(entryState);
    }
  };
  // Specialization
  template<class Event>
  struct TriggerExecutor<Event, 0> {

    static DispatchResult<StateType> execute(StateType* activeState, const Event* ev) {
      // End of recursion.
      using FirstTransition = typename TypeAt<Transitions, 0>::Result;
      using FromType = typename FirstTransition::FromType::ObjectType;
      using EventType = typename FirstTransition::EventType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = is_same<typename FirstTransition::EventType, Event>().value && hasSameFromState;
      if (conditionMet) {
        const EventType* currentTransitionEvent = reinterpret_cast<const EventType*>(ev);
        const auto result = FirstTransition().dispatch(activeState, *currentTransitionEvent);
        return result;
      }
      return DispatchResult<StateType>(false, nullptr);
    }

    static void entry(StateType* entryState) {
      // End of recursion.
      using FirstTransition = typename TypeAt<Transitions, 0>::Result;
      using ToType = typename FirstTransition::ToType::ObjectType;
      using CreationPolicy = typename FirstTransition::CreationPolicyType;

      const bool hasSameFromState = entryState->template typeOf<ToType>();

      const bool conditionMet = is_same<typename FirstTransition::EventType, Event>().value && hasSameFromState;
      if (conditionMet) {
        ToType* state = static_cast<ToType*>(entryState);
        state->template _entry<Event>();
        NullType nu;
        state->_doit(nu);
      }
      return;
    }
  };

  template<class T, class Event, int Index>
  struct Initializer {

    static DispatchResult<StateType> init() {
      using CurrentTransition = typename TypeAt<T, Index>::Result;
      using EventType = typename CurrentTransition::EventType;
      
      if (CurrentTransition::E && is_same<EventType, Event>().value) {
        EventType ev;
        auto result = CurrentTransition().dispatch(nullptr, ev);
        if (result.consumed) {
          return result;
        }
      }
      // Recursion
      return Initializer< T, Event, Index - 1 >::init();
    }
  };
  // Specialization
  template<class T, class Event>
  struct Initializer<T, Event, 0> {

    static DispatchResult<StateType> init() {
      // End of recursion.
      using FirstTransition = typename TypeAt<T, 0>::Result;
      using EventType = typename FirstTransition::EventType;

      if (FirstTransition::E && is_same<EventType, Event>().value) {
        EventType ev;
        const auto result = FirstTransition().dispatch(nullptr, ev);
        if (result.consumed) {
          return result;
        }
      }
      return Initialtransition().dispatch();
    }
  };

  template<class T, int Index>
  struct Finalizer {

    static DispatchResult<StateType> end(StateType* activeState) {
      using CurrentTransition = typename TypeAt<T, Index>::Result;
      using FromType = typename CurrentTransition::FromType::ObjectType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();
      const bool conditionMet = CurrentTransition::X && hasSameFromState;
      if (conditionMet) {
        using ET = typename CurrentTransition::EventType;
        // TODO: Add event to _exit()?
        ET ev{};
        const auto result = CurrentTransition().dispatch(activeState, ev);
        if (result.consumed) {
          return result;
        }
      }
      // Recursion
      return Finalizer< T, Index - 1 >::end(activeState);
    }
  };
  // Specialization
  template<class T>
  struct Finalizer<T, 0> {

    static DispatchResult<StateType> end(StateType* activeState) {
      // End of recursion.
      using FirstTransition = typename TypeAt<T, 0>::Result;
      using FromType = typename FirstTransition::FromType::ObjectType;

      bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = FirstTransition::X && hasSameFromState;
      if (conditionMet) {
        using ET = typename FirstTransition::EventType;
        // TODO: Add event to _exit()?
        ET ev{};
        const auto result = FirstTransition().dispatch(activeState, ev);
        if (result.consumed) {
          return result;
        }
      }
      return DispatchResult<StateType>(false, activeState);
    }
  };
};
}
