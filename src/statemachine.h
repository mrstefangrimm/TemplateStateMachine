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
#include "eventdispatchers.h"

namespace tsmlib {

template<class Transitions, class Initialtransition>
class Statemachine {
public:
  using StatePolicy = typename Initialtransition::StatePolicy;

  DispatchResult<StatePolicy> begin() {
    const auto result = Initialtransition().dispatch();
    if (result.consumed) {
      activeState_ = result.activeState;
    }
    return result;
  }

  // TODO: private: friend class SubstatesHolderState<...; instead of using "_"
  template<class Event>
  DispatchResult<StatePolicy> _begin() {

    // Transitions can have initial transitions (for a higher-level state to a sub-state).
    // The default initial transition is added to the front and is therefore executed when no other was found.
    const int size = LokiLight::Length<Transitions>::value;
    const auto result = Initializer< Transitions, Initialtransition, Event, size - 1 >::init();
    if (result.consumed) {
      activeState_ = result.activeState;
    }
    return result;
  }

  /**
      End requires an exit-transition, otherwise exit is not called.
    */
  DispatchResult<StatePolicy> end() {

    if (activeState_ == nullptr) return DispatchResult<StatePolicy>::null;

    const int size = LokiLight::Length<Transitions>::value;
    auto result = Finalizer< Transitions, size - 1 >::end(activeState_);
    if (result.consumed) {
      activeState_ = 0;
    }
    return result;
  }

  template<class Event>
  DispatchResult<StatePolicy> _end() {
    const int size = LokiLight::Length<Transitions>::value;
    const auto result = Finalizer< Transitions, size - 1 >::end(activeState_);
    if (result.consumed) {
      activeState_ = 0;
    }
    return result;
  }

  template<class Event>
  DispatchResult<StatePolicy> dispatch() {
    return dispatch(Event{});
  }

  template<class Event>
  DispatchResult<StatePolicy> dispatch(const Event& ev) {

    if (activeState_ == nullptr) return DispatchResult<StatePolicy>::null;

    const int size = LokiLight::Length<Transitions>::value;
    auto result = EventDispatcher< Transitions, Event, size - 1 >::execute(activeState_, &ev);

    // Transition not found, active state is not changed
    if (!result.consumed) {
      return DispatchResult<StatePolicy>(false, activeState_);
    }

    activeState_ = result.activeState;
    return DispatchResult<StatePolicy>(true, activeState_);
  }

private:
  StatePolicy* activeState_ = 0;
};
}
