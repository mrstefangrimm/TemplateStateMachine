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

template<uint8_t TRIGGER, typename STATE, typename TO_TRUE, typename TO_FALSE, typename FROM, typename GUARD, typename ACTION>
struct Choice {

  Choice() {
    // Choice without guard does not make sense; the state machine would immediately go to the final state.
    CTAssert < !is_same<EmptyGuard, GUARD>().value > ();
  }

  STATE* trigger(STATE* activeState) {
    typedef typename TO_TRUE::CreatorType ToTrueFactory;
    typedef typename TO_FALSE::CreatorType ToFalseFactory;
    typedef typename FROM::CreatorType FromFactory;
    FROM* fromState = FromFactory::Create();

    // The transition is valid if the "fromState" is also the activeState state from the state machine.
    if (!fromState->equals(*activeState)) {
      FromFactory::Delete(fromState);
      return activeState;
    }
    FromFactory::Delete(fromState);

    if (GUARD().check(static_cast<FROM*>(activeState))) {

      // Internal transition
      if (is_same<TO_TRUE, FROM>().value) {
        if (!is_same<ACTION, EmptyAction>().value) {
          ACTION().perform(static_cast<FROM*>(activeState));
        }
        static_cast<TO_TRUE*>(activeState)->doit(TRIGGER);
        return activeState;
      }

      static_cast<FROM*>(activeState)->exit();

      if (!is_same<ACTION, EmptyAction>().value) {
        ACTION().perform(static_cast<FROM*>(activeState));
      }
      TO_TRUE* toState = ToTrueFactory::Create();
      toState->entry();
      toState->doit(TRIGGER);
      FromFactory::Delete(static_cast<FROM*>(activeState));
      return toState;
    }

    // Internal transition
    if (is_same<TO_FALSE, FROM>().value) {
      if (!is_same<ACTION, EmptyAction>().value) {
        ACTION().perform(static_cast<FROM*>(activeState));
      }
      static_cast<TO_FALSE*>(activeState)->doit(TRIGGER);
      return activeState;
    }

    static_cast<FROM*>(activeState)->exit();

    if (!is_same<ACTION, EmptyAction>().value) {
      ACTION().perform(static_cast<FROM*>(activeState));
    }
    TO_FALSE* toState = ToFalseFactory::Create();
    toState->entry();
    toState->doit(TRIGGER);
    FromFactory::Delete(static_cast<FROM*>(activeState));
    return toState;
  }
};
}
