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
#include "transition.h"

namespace tsmlib {

template<uint8_t Trigger, typename StateType, typename TO_TRUE, typename TO_FALSE, typename FROM, typename GUARD, typename ACTION>
struct Choice {

  Choice() {
    // Choice without guard does not make sense; the state machine would immediately go to the final state.
    CompileTimeError < !is_same<GUARD, OkGuard>().value > ();
  }

  StateType* dispatch(StateType* activeState) {
    typedef typename TO_TRUE::CreatorType ToTrueFactory;
    typedef typename TO_FALSE::CreatorType ToFalseFactory;
    typedef typename FROM::CreatorType FromFactory;
    FROM* fromState = FromFactory::create();

    // The transition is valid if the "fromState" is also the activeState state from the state machine.
    if (!fromState->equals(*activeState)) {
      FromFactory::destroy(fromState);
      return activeState;
    }
    FromFactory::destroy(fromState);

    if (GUARD().eval(static_cast<FROM*>(activeState))) {

      // Self transition
      if (is_same<TO_TRUE, FROM>().value) {
        ACTION().perform(static_cast<FROM*>(activeState));
        static_cast<TO_TRUE*>(activeState)->template _doit<Trigger>();
        return activeState;
      }

      static_cast<FROM*>(activeState)->template _exit<Trigger>();

      ACTION().perform(static_cast<FROM*>(activeState));
      TO_TRUE* toState = ToTrueFactory::create();
      toState->template _entry<0>();
      toState->template _doit<Trigger>();
      FromFactory::destroy(static_cast<FROM*>(activeState));
      return toState;
    }

    // Self transition
    if (is_same<TO_FALSE, FROM>().value) {
      ACTION().perform(static_cast<FROM*>(activeState));
      static_cast<TO_FALSE*>(activeState)->template _doit<Trigger>();
      return activeState;
    }

    static_cast<FROM*>(activeState)->template _exit<Trigger>();

    ACTION().perform(static_cast<FROM*>(activeState));
    TO_FALSE* toState = ToFalseFactory::create();
    toState->template _entry<0>();
    toState->template _doit<Trigger>();
    FromFactory::destroy(static_cast<FROM*>(activeState));
    return toState;
  }
};
}
