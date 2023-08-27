#pragma once

#include "transition.h"
#include "initialtransition.h"
#include "finaltransition.h"
#include "lokilight.h"

namespace tsmlib {

  template<class Transitions, class Event, int Index>
  struct EventDispatcher {

    using StateType = typename TypeAt<Transitions, 0>::Result::StateType;

    static DispatchResult<StateType> execute(StateType* activeState, const Event* ev) {
      // Finds last element in the list that meets the conditions.
      using CurrentTransition = typename TypeAt<Transitions, Index>::Result;
      using FromType = typename CurrentTransition::FromType::ObjectType;
      using EventType = typename CurrentTransition::EventType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = is_same<EventType, Event>().value&& hasSameFromState;
      if (conditionMet) {
        const EventType* currentTransitionEvent = reinterpret_cast<const EventType*>(ev);
        auto result = CurrentTransition().dispatch(activeState, *currentTransitionEvent);
        return result;
      }
      // Recursion
      return EventDispatcher< Transitions, Event, Index - 1 >::execute(activeState, ev);
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
      EventDispatcher<StateType, Transitions, Event, Index - 1 >::entry(entryState);
    }
  };
  // Specialization
  template<class Transitions, class Event>
  struct EventDispatcher<Transitions, Event, 0> {

    using StateType = typename TypeAt<Transitions, 0>::Result::StateType;

    static DispatchResult<StateType> execute(StateType* activeState, const Event* ev) {
      // End of recursion.
      using FirstTransition = typename TypeAt<Transitions, 0>::Result;
      using FromType = typename FirstTransition::FromType::ObjectType;
      using EventType = typename FirstTransition::EventType;

      const bool hasSameFromState = activeState->template typeOf<FromType>();

      const bool conditionMet = is_same<typename FirstTransition::EventType, Event>().value&& hasSameFromState;
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

      const bool conditionMet = is_same<typename FirstTransition::EventType, Event>().value&& hasSameFromState;
      if (conditionMet) {
        ToType* state = static_cast<ToType*>(entryState);
        state->template _entry<Event>();
        NullType nu;
        state->_doit(nu);
      }
      return;
    }
  };

  template<class Transitions, class Initialtransition, class Event, int Index>
  struct Initializer {

    using StateType = typename Initialtransition::StateType;

    static DispatchResult<StateType> init() {
      using CurrentTransition = typename TypeAt<Transitions, Index>::Result;
      using EventType = typename CurrentTransition::EventType;

      if (CurrentTransition::E && is_same<EventType, Event>().value) {
        EventType ev;
        auto result = CurrentTransition().dispatch(nullptr, ev);
        if (result.consumed) {
          return result;
        }
      }
      // Recursion
      return Initializer<Transitions, Initialtransition, Event, Index - 1 >::init();
    }
  };
  // Specialization
  template<class Transitions, class Initialtransition, class Event>
  struct Initializer<Transitions, Initialtransition, Event, 0> {

    using StateType = typename Initialtransition::StateType;

    static DispatchResult<StateType> init() {
      // End of recursion.
      using FirstTransition = typename TypeAt<Transitions, 0>::Result;
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

  template<class Transitions, int Index>
  struct Finalizer {

    using StateType = typename TypeAt<Transitions, 0>::Result::StateType;

    static DispatchResult<StateType> end(StateType* activeState) {
      using CurrentTransition = typename TypeAt<Transitions, Index>::Result;
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
      return Finalizer< Transitions, Index - 1 >::end(activeState);
    }
  };
  // Specialization
  template<class Transitions>
  struct Finalizer<Transitions, 0> {

    using StateType = typename TypeAt<Transitions, 0>::Result::StateType;

    static DispatchResult<StateType> end(StateType* activeState) {
      // End of recursion.
      using FirstTransition = typename TypeAt<Transitions, 0>::Result;
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
}

