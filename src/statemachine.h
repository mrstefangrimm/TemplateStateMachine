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
  typedef typename Initialtransition::StateType StateType;

  DispatchResult<StateType> begin() {
    const auto result = Initialtransition().dispatch(nullptr);
    if (result.consumed) {
      activeState_ = result.activeState;
    }
    return result;
  }

  // TODO: private: friend class SubstatesHolderState<...; instead of using "_"
  template<class Event>
  DispatchResult<StateType> _begin() {

    // Transitions can have initaltransitions (for a higher-level state to a substate).
    // The default Initialtransition is added to the front and is therefore executed when no other was found.
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

    if (activeState_ == nullptr) return DispatchResult<StateType>::null;

    const int size = Length<Transitions>::value;
    auto result = TriggerExecutor< Event, size - 1 >::execute(activeState_);
    // Transition not found
    if (!result.consumed || result.activeState == nullptr) return DispatchResult<StateType>(false, activeState_);

    activeState_ = result.activeState;
    return DispatchResult<StateType>(true, activeState_);
  }

private:
  StateType* activeState_ = 0;

public:
  template<class Event, int Index>
  struct TriggerExecutor {
    static DispatchResult<StateType> execute(StateType* activeState) {
      // Finds last element in the list that meets the conditions.
      typedef typename TypeAt<Transitions, Index>::Result CurrentTransition;
      typedef typename CurrentTransition::FromType::ObjectType FromType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = is_same<typename CurrentTransition::EventType, Event>().value && hasSameFromState;
      if (conditionMet) {
        auto result = CurrentTransition().dispatch(activeState);
        return result;
      }
      // Recursion
      return TriggerExecutor< Event, Index - 1 >::execute(activeState);
    }

    static void entry(StateType* entryState) {
      // Finds last element in the list that meets the conditions.
      typedef typename TypeAt<Transitions, Index>::Result CurrentTransition;
      typedef typename CurrentTransition::ToType::ObjectType ToType;
      typedef typename CurrentTransition::CreationPolicyType CreationPolicy;

      // TODO: Mustn't be a choice transition
      //CompileTimeError < is_same<ToType, EmptyState<typename CreationPolicy::ObjectType>>().value >();

      bool hasSameFromState = entryState->template typeOf<ToType>();

      bool conditionMet = is_same<typename CurrentTransition::EventType, Event>().value&& hasSameFromState;
      if (conditionMet) {
        ToType* state = static_cast<ToType*>(entryState);
        state->template _entry<Event>();
        state->template _doit<Event>();
        return;
      }
      // Recursion
      TriggerExecutor< Event, Index - 1 >::entry(entryState);
    }
  };
  // Specialization
  template<class Event>
  struct TriggerExecutor<Event, 0> {
    static DispatchResult<StateType> execute(StateType* activeState) {
      // End of recursion.
      typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
      typedef typename FirstTransition::FromType::ObjectType FromType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = is_same<typename FirstTransition::EventType, Event>().value && hasSameFromState;
      if (conditionMet) {
        const auto result = FirstTransition().dispatch(activeState);
        return result;
      }
      return DispatchResult<StateType>(false, nullptr);
    }
    static void entry(StateType* entryState) {
      // End of recursion.
      typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
      typedef typename FirstTransition::ToType::ObjectType ToType;
      typedef typename FirstTransition::CreationPolicyType CreationPolicy;

      const bool hasSameFromState = entryState->template typeOf<ToType>();

      const bool conditionMet = is_same<typename FirstTransition::EventType, Event>().value && hasSameFromState;
      if (conditionMet) {
        ToType* state = static_cast<ToType*>(entryState);
        state->template _entry<Event>();
        state->template _doit<Event>();
      }
      return;
    }
  };

  template<class T, class Event, int Index>
  struct Initializer {
    static DispatchResult<StateType> init() {
      typedef typename TypeAt<T, Index>::Result CurrentTransition;
      if (CurrentTransition::E && is_same<typename CurrentTransition::EventType, Event>().value) {
        auto result = CurrentTransition().dispatch(nullptr);
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
      typedef typename TypeAt<T, 0>::Result FirstTransition;
      if (FirstTransition::E && is_same<typename FirstTransition::EventType, Event>().value) {
        const auto result = FirstTransition().dispatch(nullptr);
        if (result.consumed) {
          return result;
        }
      }
      return Initialtransition().dispatch(nullptr);
    }
  };

  template<class T, int Index>
  struct Finalizer {
    static DispatchResult<StateType> end(StateType* activeState) {
      typedef typename TypeAt<T, Index>::Result CurrentTransition;
      typedef typename CurrentTransition::FromType::ObjectType FromType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();
      const bool conditionMet = CurrentTransition::X && hasSameFromState;
      if (conditionMet) {
        const auto result = CurrentTransition().dispatch(activeState);
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
      typedef typename TypeAt<T, 0>::Result FirstTransition;
      typedef typename FirstTransition::FromType::ObjectType FromType;

      bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = FirstTransition::X && hasSameFromState;
      if (conditionMet) {
        const auto result = FirstTransition().dispatch(activeState);
        if (result.consumed) {
          return result;
        }
      }
      return DispatchResult<StateType>(false, activeState);
    }
  };
};
}
