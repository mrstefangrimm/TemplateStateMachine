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

// TODO: Superstate not used
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

    template<uint8_t T>
    DispatchResult<StateType> dispatch() {

      if (_activeState == 0) return DispatchResult<StateType>(false, _activeState);

      const int size = Length<Transitions>::value;
      StateType* state = TriggerExecutor < Transitions, size - 1, T, StateType >::execute(_activeState);
      // Transition not found
      if (state == 0) return DispatchResult<StateType>(false, _activeState);

      _activeState = state;
      return DispatchResult<StateType>(true, _activeState);
    }

  private:
    StateType* _activeState = 0;

  public:
    template<typename Typelist, int Index, uint8_t N, typename O>
    struct TriggerExecutor {

      static StateType* execute(O* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Typelist, Index>::Result CurentTransition;
        typedef typename CurentTransition::FromType::CreatorType FromFactory;
        typedef typename CurentTransition::FromType::ObjectType FromFactoryType;
        O* fromState = FromFactory::create();
        // fromState is 0 for AnyState
        bool hasSameFromState = fromState != 0 ? activeState->equals(*fromState) : true;
        FromFactory::destroy(static_cast<FromFactoryType*>(fromState));

        bool conditionMet = CurentTransition::N == N && hasSameFromState;
        if (conditionMet) {
          if (CurentTransition::E) {
            return Endtransition().dispatch(activeState).activeState;
          }
          auto result = CurentTransition().dispatch(activeState);
          // If the state has not changed, continue to see if any other transition does.
          if (result.consumed) {
            return result.activeState;
          }
        }
        // Recursion
        StateType* resState = TriggerExecutor < Typelist, Index - 1, N, O >::execute(activeState);
        if (resState != 0) {
          return resState;
        }
        return 0;
      }
    };

    template<typename Typelist, uint8_t N, typename O>
    struct TriggerExecutor<Typelist, 0, N, O> {
      static StateType* execute(O* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
        typedef typename FirstTransition::FromType::CreatorType FromFactory;
        typedef typename FirstTransition::FromType::ObjectType FromFactoryType;
        O* fromState = FromFactory::create();
        // fromState is 0 for AnyState
        bool hasSameFromState = fromState != 0 ? activeState->equals(*fromState) : true;
        FromFactory::destroy(static_cast<FromFactoryType*>(fromState));

        bool conditionMet = FirstTransition::N == N && hasSameFromState;
        if (conditionMet) {
          if (FirstTransition::E) {
            return Endtransition().dispatch(activeState).activeState;
          }
          return FirstTransition().dispatch(activeState).activeState;
        }
        return 0;
      }
    };
};
}
