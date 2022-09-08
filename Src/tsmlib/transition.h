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

  using namespace Loki;
  using namespace std;

  template<typename State>
  struct TriggerResult {
    TriggerResult(bool changed, State* state) {
      consumed = changed;
      activeState = state;
    }
    bool consumed;
    State* activeState;
  };

  template<typename State>
  struct EmptyState : State {
    typedef EmptyState CreatorType;

    static EmptyState* Create() { return 0; }
    static void Delete(EmptyState*) { }

    void entry() { }
    void exit() { }
    template<uint8_t N>
    void doit() { }
  };

  /* Provides Action interface and does nothing. */
  struct EmptyAction {
    template<typename T>
    void perform(T*) { }
  };

  /* Provides Guard interface and returns true. */
  struct OkGuard {
    template<typename T>
    bool check(T*) { return true; }
  };

  template<uint8_t TRIGGER, typename STATE, typename TO, typename FROM, typename GUARD, typename ACTION, bool Exit>
  struct Transition {
    enum { Trigger = TRIGGER };
    enum { ExitingTransition = Exit };
    typedef FROM FromType;

    TriggerResult<STATE> trigger(STATE* activeState) {
      typedef typename TO::CreatorType ToFactory;
      typedef typename FROM::CreatorType FromFactory;
      TO* toState = ToFactory::Create();
      FROM* fromState = FromFactory::Create();

      // Initial transition
      if (!is_same<EmptyState<STATE>, TO>().value && is_same<EmptyState<STATE>, FROM>().value) {
        ACTION().perform(static_cast<FROM*>(activeState));
        toState->entry();
        toState->template doit<TRIGGER>();

        // Delete not needed. "activeState" and "fromState" are null (the initial state)

        return TriggerResult<STATE>(true, toState);
      }

      // Final transition Any <- Any
      if (is_same<AnyState<STATE>, FROM>().value) {

        // Delete toState and fromState not needed; both are "null".

        if (GUARD().check(static_cast<FROM*>(activeState))) {
          // TODO: "exit" of AnyState is called, not from the activeState object. Polymorphism is required.
          static_cast<FROM*>(activeState)->exit();
          ACTION().perform(static_cast<FROM*>(activeState));
          // TODO: AnyState::Delete is called
          FromFactory::Delete(static_cast<FROM*>(activeState));
          return TriggerResult<STATE>(true, toState);
        }
        return TriggerResult<STATE>(false, activeState);
      }

      // The transition is valid if the "fromState" is also the activeState state from the state machine.
      if (!fromState->equals(*activeState)) {
        ToFactory::Delete(toState);
        FromFactory::Delete(fromState);
        return TriggerResult<STATE>(false, activeState);
      }
      FromFactory::Delete(fromState);

      if (!GUARD().check(static_cast<FROM*>(activeState))) {
        ToFactory::Delete(toState);
        return TriggerResult<STATE>(false, activeState);
      }
      // Self transition
      if (is_same<TO, FROM>().value) {

        ACTION().perform(static_cast<FROM*>(activeState));
        static_cast<TO*>(activeState)->template doit<TRIGGER>();
        ToFactory::Delete(toState);
        return TriggerResult<STATE>(true, activeState);
      }

      static_cast<FROM*>(activeState)->exit();

      ACTION().perform(static_cast<FROM*>(activeState));
      toState->entry();
      toState->template doit<TRIGGER>();
      FromFactory::Delete(static_cast<FROM*>(activeState));
      return TriggerResult<STATE>(true, toState);
    }
  };

  template<typename State, typename To, typename Action>
  struct InitialTransition : Transition<0, State, To, EmptyState<State>, OkGuard, Action, false> {
  };

  template<typename STATE, typename GUARD, typename ACTION>
  struct FinalTransition : Transition<0, STATE, EmptyState<STATE>, AnyState<STATE>, GUARD, ACTION, false> {
    typedef GUARD GuardType;
    //enum { ExitingTransition = true };

    FinalTransition() {
      // Final transition without guard does not make sense; the state machine would immediately go to the final state.
      CompileTimeError < !is_same<GUARD, OkGuard>().value >();
    }
  };

  template<typename STATE>
  struct NullFinalTransition : Transition<0, STATE, EmptyState<STATE>, AnyState<STATE>, OkGuard, EmptyAction, false> {
    typedef OkGuard GuardType;
  };

}
