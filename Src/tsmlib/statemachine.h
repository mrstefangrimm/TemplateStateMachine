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
#include "templatemeta.h"
#include "transition.h"

namespace tsmlib {

template<typename STATE>
struct NullStatemachine {
  STATE* trigger() {
    return 0;
  }
  static NullStatemachine<STATE>* Instance;
};
template<typename T> NullStatemachine<T>* NullStatemachine<T>::Instance = 0;

template<typename STATE, typename TRANSITIONS, typename INNERSM, typename INITIALTRANSITION, typename FINALTRANSITION>
class Statemachine {
  public:

    Statemachine(bool immediatelyBegin = false) {
      if (immediatelyBegin) begin();
    }

    void begin() {
      _activeState = INITIALTRANSITION().trigger(0);
    }

    void end() {
      if (_activeState != 0) FINALTRANSITION().trigger(_activeState);
    }

    template<uint8_t T>
    STATE* trigger() {

      if (_activeState == 0) return _activeState;

      // Final transition. Check guard and terminate.
      //if (!is_same<NullFinalTransition<STATE>, FINALTRANSITION>().value) {
      //  typename FINALTRANSITION::GuardType guard;
      //  if (guard.check()) {
      //    //typedef typename TypeAt<STATES, _activeStateIdx>::Result ActiveState;
      //    //ActiveState activeState;
      //    //activeState.exit();
      //    _activeState = FINALTRANSITION().trigger(_activeState);
      //    if (_activeState == 0) return _activeState;
      //  }
      //}

      const int size = Length<TRANSITIONS>::value;
      STATE* state = TriggerExecutor < TRANSITIONS, size - 1, T, STATE >::execute(_activeState);
      // Transition not found
      if (state == 0) return _activeState;

      _activeState = state;
      return _activeState;
    }
  private:
    STATE* _activeState = 0;

  public:
    template<typename TL, int INDEX, uint8_t TRIGGER, typename FROM>
    struct TriggerExecutor {

      static STATE* execute(FROM* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<TL, INDEX>::Result CurentElement;

        typedef typename CurentElement::FromType::CreatorType FromFactory;
        typedef typename CurentElement::FromType::ObjectType FromFactoryType;
        FROM* fromState = FromFactory::Create();
        bool hasSameFromState = activeState->equals(*fromState);
        FromFactory::Delete(static_cast<FromFactoryType*>(fromState));

        bool conditionMet = CurentElement::Trigger == TRIGGER && hasSameFromState;
        if (conditionMet) {
          return CurentElement().trigger(activeState);
        }
        // Recursion
        STATE* resState = TriggerExecutor < TL, INDEX - 1, TRIGGER, FROM >::execute(activeState);
        if (resState != 0) {
          return resState;
        }
        return 0;
      }
    };

    template<typename TL, uint8_t TRIGGER, typename FROM>
    struct TriggerExecutor<TL, 0, TRIGGER, FROM> {
      static STATE* execute(FROM* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<TL, 0>::Result FirstElement;

        typedef typename FirstElement::FromType::CreatorType FromFactory;
        typedef typename FirstElement::FromType::ObjectType FromFactoryType;
        FROM* fromState = FromFactory::Create();
        bool hasSameFromState = activeState->equals(*fromState);
        FromFactory::Delete(static_cast<FromFactoryType*>(fromState));

        bool conditionMet = FirstElement::Trigger == TRIGGER && hasSameFromState;
        if (conditionMet) {
          return FirstElement().trigger(activeState);
        }
        return 0;
      }
    };
};
}
