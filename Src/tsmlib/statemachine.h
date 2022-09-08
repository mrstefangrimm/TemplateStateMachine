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

template<typename STATE, typename TRANSITIONS, typename INITIALTRANSITION, typename FINALTRANSITION>
class Statemachine {
  public:

    Statemachine(bool immediatelyBegin = false) {
      if (immediatelyBegin) begin();
    }

    void begin() {
      _activeState = INITIALTRANSITION().trigger(0).activeState;
    }

    void end() {
      FINALTRANSITION().trigger(_activeState);
    }

    template<uint8_t T>
    TriggerResult<STATE> trigger() {

      if (_activeState == 0) return TriggerResult<STATE>(false, _activeState);

      const int size = Length<TRANSITIONS>::value;
      STATE* state = TriggerExecutor < TRANSITIONS, size - 1, T, STATE >::execute(_activeState);
      // Transition not found
      if (state == 0) return TriggerResult<STATE>(false, _activeState);;

      _activeState = state;
      return TriggerResult<STATE>(true, _activeState);;
    }

  private:
    STATE* _activeState = 0;

  public:
    template<typename TL, int INDEX, uint8_t TRIGGER, typename FROM>
    struct TriggerExecutor {

      static STATE* execute(FROM* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<TL, INDEX>::Result CurentTransition;
        typedef typename CurentTransition::FromType::CreatorType FromFactory;
        typedef typename CurentTransition::FromType::ObjectType FromFactoryType;
        FROM* fromState = FromFactory::Create();
        // fromState is 0 for AnyState
        bool hasSameFromState = fromState != 0 ? activeState->equals(*fromState) :true;
        FromFactory::Delete(static_cast<FromFactoryType*>(fromState));

        bool conditionMet = CurentTransition::Trigger == TRIGGER && hasSameFromState;
        if (conditionMet) {
          if (CurentTransition::ExitingTransition) {
            return FINALTRANSITION().trigger(activeState).activeState;
          }
          auto result = CurentTransition().trigger(activeState);
          // If the state has not changed, continue to see if any other transition does.
          if (result.consumed) {
            return result.activeState;
          }
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
        typedef typename TypeAt<TL, 0>::Result FirstTransition;
        typedef typename FirstTransition::FromType::CreatorType FromFactory;
        typedef typename FirstTransition::FromType::ObjectType FromFactoryType;
        FROM* fromState = FromFactory::Create();
        // fromState is 0 for AnyState
        bool hasSameFromState = fromState != 0 ? activeState->equals(*fromState) : true;
        FromFactory::Delete(static_cast<FromFactoryType*>(fromState));

        bool conditionMet = FirstTransition::Trigger == TRIGGER && hasSameFromState;
        if (conditionMet) {
          if (FirstTransition::ExitingTransition) {
            return FINALTRANSITION().trigger(activeState).activeState;
          }
          return FirstTransition().trigger(activeState).activeState;
        }
        return 0;
      }
    };
};
}
