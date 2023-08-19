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

template<typename Me, typename CreationPolicy>
struct FinalTransition {

  enum { N = -1 };
  enum { E = false };
  enum { X = true };

  typedef typename CreationPolicy::ObjectType StateType;
  typedef Me FromType;

  DispatchResult<StateType> dispatch(StateType* activeState) {
    typedef typename Me::CreatorType FromFactory;
    typedef typename CreationPolicy::CreatorType Creator;

    static_cast<Me*>(activeState)->template _exit<N>();

    FromFactory::destroy(static_cast<Me*>(activeState));

    return DispatchResult<StateType>(true, nullptr);
  }
};

template<uint8_t Trigger, typename From, typename CreationPolicy, typename Guard, typename Action>
struct FinalTransitionExplicit : impl::TransitionBase<Trigger, EmptyState<typename CreationPolicy::ObjectType>, From, CreationPolicy, Guard, Action, false, false, false> {
  FinalTransitionExplicit() {
    // To Make sure the user defines a guard for the final transition. This is not UML compliant.
    CompileTimeError< !is_same<Guard, NoGuard>().value >();
  }
};
}
