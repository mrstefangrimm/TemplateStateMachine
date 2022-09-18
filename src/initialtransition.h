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

template<typename To, typename CreationPolicy, typename Action>
struct InitialTransition {

  enum { N = -1 };
  enum { E = true };
  enum { X = false };
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
    bool cosumedBySubstate = toState->template _entry<N>();
    if (!cosumedBySubstate) {
      toState->template _doit<N>();
    }
    return DispatchResult<StateType>(true, toState);
  }
};
}
