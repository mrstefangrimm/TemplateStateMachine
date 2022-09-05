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

  template<typename STATE>
  struct EmptyState : STATE {
    typedef EmptyState CreatorType;
    void entry() { }
    void exit() { }
    void doit() { }
    static EmptyState* Create() { return 0; }
    static void Delete(EmptyState*) { }
  };

  template<typename STATE>
  struct AnyState : STATE {
  };

  struct EmptyAction {
    void operator()() {
    }
  };

  struct EmptyGuard {
    bool check() {
      return false;
    }
  };

  template<uint8_t TRIGGER, typename STATE, typename TO, typename FROM, typename GUARD, typename ACTION>
  struct Transition {
    //typedef TO ToType;

    STATE* trigger(STATE* current) {
      typedef typename TO::CreatorType ToFactory;
      typedef typename FROM::CreatorType FromFactory;
      TO* toState = ToFactory::Create();
      FROM* fromState = FromFactory::Create();

      // Initial transition
      if (!is_same<EmptyState<STATE>, TO>().value && is_same<EmptyState<STATE>, FROM>().value) {
        ACTION()();
        toState->entry();
        toState->doit();

        // Delete not needed. "current" and "fromState" are null (the initial state)

        return toState;
      }

      // Final transition
      //if (is_same<EmptyState<STATE>, TO>().value && is_same<AnyState<STATE>, FROM>().value) {

      //  current->Exit();
      //  ACTION()();

      //  return toState;
      //}

      // The transition is valid if the "fromState" is also the current state from the state machine.
      if (!fromState->equals(*current)) {
        ToFactory::Delete(toState);
        FromFactory::Delete(fromState);
        return current;
      }

      if (!is_same<GUARD, EmptyGuard>().value) {
        GUARD guard;
        if (!guard.check()) {         
          ToFactory::Delete(toState);
          FromFactory::Delete(fromState);
          return current;
        }
      }
      // Internal transition
      if (is_same<TO, FROM>().value) {

        if (!is_same<ACTION, EmptyAction>().value) {
          ACTION()();
        }
        static_cast<TO*>(current)->doit();
        ToFactory::Delete(toState);
        FromFactory::Delete(fromState);
        return current;
      }

      fromState->exit();

      if (!is_same<ACTION, EmptyAction>().value) {
        ACTION()();
      }
      toState->entry();
      toState->doit();
      // TODO: cleanup Do(Int2Type<TRIGGER>()) - toState->Do(Int2Type<TRIGGER>());
      FromFactory::Delete(static_cast<FROM*>(current));
      FromFactory::Delete(fromState);
      return toState;
    }
  };

  template<typename STATE, typename TO, typename ACTION>
  struct InitialTransition : Transition<0, STATE, TO, EmptyState<STATE>, EmptyGuard, ACTION> { };

  template<typename STATE, typename GUARD, typename ACTION>
  struct FinalTransition : Transition<0, STATE, EmptyState<STATE>, AnyState<STATE>, GUARD, ACTION> {
    typedef GUARD GuardType;

    FinalTransition() {
      // Final transition without guard does not make sense; the state machine would immediately go to the final state.
      CTAssert<!is_same<EmptyGuard, GUARD>().value>();
    }
  };

  template<typename STATE>
  struct NullFinalTransition : Transition<0, STATE, EmptyState<STATE>, AnyState<STATE>, EmptyGuard, EmptyAction> {
    typedef EmptyGuard GuardType;
  };

}
