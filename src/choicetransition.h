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

template<uint8_t Trigger, typename To_true, typename To_false, typename From, typename CreationPolicy, typename Check, typename Action>
struct ChoiceTransition {

    enum { N = Trigger };
    typedef To_false ToType;
    typedef From FromType;
    typedef CreationPolicy CreationPolicyType;
    typedef typename CreationPolicy::ObjectType StateType;

    ChoiceTransition() {
      // Choice without guard does not make sense; the choice is either to go the the state To_true or To_false.
      CompileTimeError < !is_same<Check, OkGuard>().value > ();
      // Choice transition
      CompileTimeError < !is_same<To_false, EmptyState<typename CreationPolicy::ObjectType>>().value > ();
    }

    DispatchResult<StateType> dispatch(StateType* activeState) {

      typedef typename To_true::CreatorType ToFactory;
      typedef typename From::CreatorType FromFactory;

      Action().perform(activeState);

      if (Check().eval(activeState)) {
        return execute< To_true >(activeState);
      }
      else {
        return execute< To_false >(activeState);
      }
    }

  private:
    template<typename To>
    DispatchResult<StateType> execute(StateType* activeState) {

      typedef typename To::CreatorType ToFactory;
      typedef typename From::CreatorType FromFactory;

      // Self transition
      if (is_same<To, From>().value) {
        static_cast<To*>(activeState)->template _doit<Trigger>();
        return DispatchResult<StateType>(true, activeState, false);
      }

      static_cast<From*>(activeState)->_exit();
      FromFactory::destroy(static_cast<From*>(activeState));

      To* toState = ToFactory::create();
      bool cosumedBySubstate = toState->template _entry<Trigger>();
      if (!cosumedBySubstate) {
        toState->template _doit<Trigger>();
      }
      return DispatchResult<StateType>(true, toState, false);
    }
};
}
