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

template<typename StateType, typename Transitions, typename Initialtransition, typename Endtransition>
class Statemachine {
  public:

    DispatchResult<StateType> begin() {
      auto result = Initialtransition().dispatch(0);
      if (result.consumed) {
        _activeState = result.activeState;
      }
      return result;
    }

    DispatchResult<StateType> end() {
      auto result = Endtransition().dispatch(_activeState);
      if (result.consumed) {
        _activeState = 0;
      }
      return result;
    }

    template<uint8_t N>
    DispatchResult<StateType> dispatch() {

      if (_activeState == 0) return DispatchResult<StateType>(false, _activeState);

      const int size = Length<Transitions>::value;
      StateType* state = TriggerExecutor < N, size - 1 >::execute(_activeState);
      // Transition not found
      if (state == 0) return DispatchResult<StateType>(false, _activeState);

      _activeState = state;
      return DispatchResult<StateType>(true, _activeState);
    }

  private:
    StateType* _activeState = 0;

  public:
    template<uint8_t N, int Index>
    struct TriggerExecutor {

      static StateType* execute(StateType* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, Index>::Result CurentTransition;
        typedef typename CurentTransition::FromState::CreatorType FromFactory;
        //typedef typename CurentTransition::FromState::ObjectType FromFactoryType;
        auto fromState = FromFactory::create();
        // fromState is 0 for AnyState
        bool hasSameFromState = fromState != 0 ? activeState->equals(*fromState) : true;
        FromFactory::destroy(fromState);

        bool conditionMet = CurentTransition::N == N && hasSameFromState;
        if (conditionMet) {
          //if (CurentTransition::E) {
          //  return Endtransition().dispatch(activeState).activeState;
          //}
          auto result = CurentTransition().dispatch(activeState);
          // If the state has not changed, continue to see if any other transition does.
          if (result.consumed) {
            return result.activeState;
          }
        }
        // Recursion
        StateType* resState = TriggerExecutor < N, Index - 1 >::execute(activeState);
        if (resState != 0) {
          return resState;
        }
        return 0;
      }
    };

    template<uint8_t N>
    struct TriggerExecutor<N, 0> {
      static StateType* execute(StateType* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
        typedef typename FirstTransition::FromState::CreatorType FromFactory;
        //typedef typename FirstTransition::FromState::ObjectType FromFactoryType;
        auto fromState = FromFactory::create();
        // fromState is 0 for AnyState
        bool hasSameFromState = fromState != 0 ? activeState->equals(*fromState) : true;
        FromFactory::destroy(fromState);

        bool conditionMet = FirstTransition::N == N && hasSameFromState;
        if (conditionMet) {
          //if (FirstTransition::E) {
          //  return Endtransition().dispatch(activeState).activeState;
          //}
          return FirstTransition().dispatch(activeState).activeState;
        }
        return 0;
      }
    };
};
}
