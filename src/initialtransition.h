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
#include "lokilight.h"

namespace tsmlib {

template<class To, class CreationPolicy, class Action>
struct InitialTransition {

  enum { E = true };
  enum { X = false };
  enum { R = false };

  typedef NullType EventType;
  typedef To ToType;
  typedef CreationPolicy CreationPolicyType;
  typedef typename CreationPolicy::ObjectType StateType;
  typedef EmptyState<StateType> FromType;

  InitialTransition() {
  }

  DispatchResult<StateType> dispatch(StateType* activeState) {
    typedef typename To::CreatorType ToFactory;

    Action().perform(activeState);
    To* toState = ToFactory::create();
    toState->template _entry<EventType>();
    if (is_base_of<BasicState<To, StateType>, To>::value) {
      toState->template _doit<EventType>();
    }
    return DispatchResult<StateType>(true, toState);
  }

  DispatchResult<StateType> dispatch() {
    typedef typename To::CreatorType ToFactory;

    Action().perform();
    To* toState = ToFactory::create();
    toState->template _entry<EventType>();
    if (is_base_of<BasicState<To, StateType>, To>::value) {

      EventType ev{};
      toState->template _doit<EventType>(ev);
    }
    return DispatchResult<StateType>(true, toState);
  }
};
}
