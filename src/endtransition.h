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

template<typename CreationPolicy, typename Guard, typename Action, bool AssertNotOkGuard>
struct EndTransitionBase {

  enum { N = -1 };
  enum { E = false };
  enum { X = true };
  typedef CreationPolicy CreationPolicyType;
  typedef typename CreationPolicy::ObjectType StateType;
  typedef EmptyState<StateType> ToType;
  typedef AnyState<StateType> FromType;

  EndTransitionBase() {
    // Final transition without guard does not make sense; the state machine would immediately go to the final state.
    CompileTimeError < !AssertNotOkGuard || (!is_same<Guard, OkGuard>().value) > ();
  }

  DispatchResult<StateType> dispatch(StateType* activeState) {

    if (Guard().eval(activeState)) {
      Action().perform(activeState);
      // TODO: "exit" of AnyState is called, not from the activeState object. Polymorphism is required.
      static_cast<StateType*>(activeState)->_exit<N>();

      typedef typename CreationPolicy::CreatorType Creator;
      typedef typename CreationPolicy::ObjectType Object;
      // TODO: AnyState::destroy is called.
      Creator::destroy(activeState);

      return DispatchResult<StateType>(true, 0);
    }
    return DispatchResult<StateType>(false, activeState);
  }
};

template<typename CreationPolicy, typename Guard, typename Action>
using EndTransition = EndTransitionBase<CreationPolicy, Guard, Action, true>;

template<typename CreationPolicy>
using NullEndTransition = EndTransitionBase<CreationPolicy, OkGuard, EmptyAction, false>;

}
