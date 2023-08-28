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

namespace tsmlib {

template<class Me>
struct FinalTransition {

  enum { E = false };
  enum { X = true };

  using EventType = NullType;
  using FromType = Me;
  using StatePolicy = typename Me::Policy;

  DispatchResult<StatePolicy> dispatch(StatePolicy* activeState) {
    using FromFactory = typename Me::CreatorType;

    static_cast<Me*>(activeState)->template _exit<EventType>();

    FromFactory::destroy(static_cast<Me*>(activeState));

    return DispatchResult<StatePolicy>(true, nullptr);
  }

  DispatchResult<StatePolicy> dispatch(StatePolicy* activeState, const EventType& ev) {
    using FromFactory = typename Me::CreatorType;

    static_cast<Me*>(activeState)->template _exit<EventType>(ev);

    FromFactory::destroy(static_cast<Me*>(activeState));

    return DispatchResult<StatePolicy>(true, nullptr);
  }
};

template<class Event, class From, class Guard, class Action>
struct FinalTransitionExplicit : impl::TransitionBase<Event, EmptyState<typename From::Policy>, From, Guard, Action, false, false, false> {
  FinalTransitionExplicit() {
    // TODO - Boost SML example is without guard.
    // To Make sure the user defines a guard for the final transition. This is not UML compliant.
    //CompileTimeError< !is_same<Guard, NoGuard>().value >();
  }
};
}
