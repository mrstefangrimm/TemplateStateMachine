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

template<typename Transitions, typename Initialtransition, typename Endtransition>
class Statemachine {
  public:
    typedef typename Initialtransition::StateType StateType;

    Statemachine() {
      typedef typename Initialtransition::StateType InitStateType;
      typedef typename Endtransition::StateType EndStateType;
      CompileTimeError<is_same<InitStateType, EndStateType>().value>();
    }

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
      auto result = TriggerExecutor<N, size-1>::execute(_activeState);
      // Transition not found
      if (result.state == 0) return DispatchResult<StateType>(false, _activeState);

      if (result.deferredEntry) {
        TriggerExecutor<N, size-1>::entry(result.state);
        _activeState = 0;
        return DispatchResult<StateType>(true, result.state, true);
      }
      _activeState = result.state;
      return DispatchResult<StateType>(true, _activeState);
    }

  private:
    StateType* _activeState = 0;

  public:

    struct ExecuteResult {
      StateType* state = 0;
      bool deferredEntry = false;
      int transitionIndex = 0;
    };
    template<uint8_t N, int Index>
    struct TriggerExecutor {
      static ExecuteResult execute(StateType* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, Index>::Result CurentTransition;
        typedef typename CurentTransition::FromType::ObjectType FromType;
        bool hasSameFromState = activeState->template typeOf<FromType>();

        bool conditionMet = CurentTransition::N == N && hasSameFromState;
        if (conditionMet) {
          auto result = CurentTransition().dispatch(activeState);
          // If the state has not changed, continue to see if any other transition does.
          if (result.consumed) {
            ExecuteResult ret;
            ret.state = result.activeState;
            ret.deferredEntry = result.deferredEntry;
            ret.transitionIndex = Index;
            return ret;
          }
        }
        // Recursion
        auto res = TriggerExecutor < N, Index - 1 >::execute(activeState);
        if (res.state != 0) {
          return res;
        }
        return ExecuteResult();
      }
      static void entry(StateType* entryState) {
        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, Index>::Result CurentTransition;
        typedef typename CurentTransition::ToType::ObjectType ToType;
        bool hasSameFromState = entryState->template typeOf<ToType>();

        bool conditionMet = CurentTransition::N == N && hasSameFromState;
        if (conditionMet) {
          ToType* state = static_cast<ToType*>(entryState);
          state->entry_();
          state->template doit_<N>();
          return;
        }
        // Recursion
        TriggerExecutor < N, Index - 1 >::entry(entryState);
      }
      static bool checkHierarchy() {
        // TODO: Idea to check the hierarchy in the Tranistions (e.g. exit declarations last, etc.)
      }
    };
    // Specialization
    template<uint8_t N>
    struct TriggerExecutor<N, 0> {
      static ExecuteResult execute(StateType* activeState) {

        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
        typedef typename FirstTransition::FromType::ObjectType FromType;
        bool hasSameFromState = activeState->template typeOf<FromType>();

        bool conditionMet = FirstTransition::N == N && hasSameFromState;
        if (conditionMet) {
          auto result = FirstTransition().dispatch(activeState);
          ExecuteResult ret;
          ret.state = result.activeState;
          ret.deferredEntry = result.deferredEntry;
          ret.transitionIndex = 0;
          return ret;
        }
        return ExecuteResult();
      }
      static void entry(StateType* entryState) {
        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
        typedef typename FirstTransition::ToType::ObjectType ToType;
        bool hasSameFromState = entryState->template typeOf<ToType>();

        bool conditionMet = FirstTransition::N == N && hasSameFromState;
        if (conditionMet) {
          ToType* state = static_cast<ToType*>(entryState);
          state->entry_();
          state->template doit_<N>();
        }
        return;
      }
    };
};
}
