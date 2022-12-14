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
      CompileTimeError<is_same<Initialtransition::StateType, typename Endtransition::StateType>().value>();
    }

    DispatchResult<StateType> begin() {
      auto result = Initialtransition().dispatch(0);
      if (result.consumed) {
        activeState_ = result.activeState;
      }
      return result;
    }

    // TODO: private: friend class SubstatesHolderState<...;
    template<uint8_t N>
    DispatchResult<StateType> _begin() {

      // Transitions can have initaltransitions (for a higher-level state to a substate).
      // The default Initialtransition is added to the front and is therefore executed when no other was found.
      const int size = Length<Transitions>::value;
      auto result = Initializer < Transitions, N, size - 1 >::init();
      if (result.consumed) {
        activeState_ = result.activeState;
      }
      return result;
    }

    DispatchResult<StateType> end() {
      auto result = Endtransition().dispatch(activeState_);
      if (result.consumed) {
        activeState_ = 0;
      }
      return result;
    }

    template<uint8_t N>
    DispatchResult<StateType> _end() {
      const int size = Length<Transitions>::value;
      auto result = Finisher < Transitions, N, size - 1 >::end(activeState_);
      if (result.consumed) {
        activeState_ = 0;
      }
      return result;
    }

    template<uint8_t N>
    DispatchResult<StateType> dispatch() {

      if (activeState_ == 0) return DispatchResult<StateType>(false, activeState_);

      const int size = Length<Transitions>::value;
      auto result = TriggerExecutor < N, size - 1 >::execute(activeState_);
      // Transition not found
      if (result.state == 0) return DispatchResult<StateType>(false, activeState_);

      if (result.deferredEntry) {
        TriggerExecutor < N, size - 1 >::entry(result.state);
        activeState_ = 0;
        return DispatchResult<StateType>(true, result.state, true);
      }
      activeState_ = result.state;
      return DispatchResult<StateType>(true, activeState_);
    }

  private:
    StateType* activeState_ = 0;

  public:

    struct ExecuteResult {
      StateType* state = 0;
      bool deferredEntry = false;
    };
    template<uint8_t N, int Index>
    struct TriggerExecutor {
      static ExecuteResult execute(StateType* activeState) {
        // Finds last element in the list that meets the conditions.
        typedef typename TypeAt<Transitions, Index>::Result CurrentTransition;
        typedef typename CurrentTransition::FromType::ObjectType FromType;

        bool hasSameFromState = activeState->template typeOf<FromType>();

        bool conditionMet = CurrentTransition::N == N && hasSameFromState;
        if (conditionMet) {
          auto result = CurrentTransition().dispatch(activeState);
          // If the state has not changed, continue to see if any other transition does.
          if (result.consumed) {
            ExecuteResult ret;
            ret.state = result.activeState;
            ret.deferredEntry = result.deferredEntry;
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
        typedef typename TypeAt<Transitions, Index>::Result CurrentTransition;
        typedef typename CurrentTransition::ToType::ObjectType ToType;
        typedef typename CurrentTransition::CreationPolicyType CreationPolicy;

        // TODO: Mustn't be a choice transition
        //CompileTimeError < is_same<ToType, EmptyState<typename CreationPolicy::ObjectType>>().value >();

        bool hasSameFromState = entryState->template typeOf<ToType>();

        bool conditionMet = CurrentTransition::N == N && hasSameFromState;
        if (conditionMet) {
          ToType* state = static_cast<ToType*>(entryState);
          state->template _entry<N>();
          state->template _doit<N>();
          return;
        }
        // Recursion
        TriggerExecutor < N, Index - 1 >::entry(entryState);
      }
    };
    // Specialization
    template<uint8_t N>
    struct TriggerExecutor<N, 0> {
      static ExecuteResult execute(StateType* activeState) {
        // End of recursion.
        typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
        typedef typename FirstTransition::FromType::ObjectType FromType;

        bool hasSameFromState = activeState->template typeOf<FromType>();

        bool conditionMet = FirstTransition::N == N && hasSameFromState;
        if (conditionMet) {
          auto result = FirstTransition().dispatch(activeState);
          ExecuteResult ret;
          ret.state = result.activeState;
          ret.deferredEntry = result.deferredEntry;
          return ret;
        }
        return ExecuteResult();
      }
      static void entry(StateType* entryState) {
        // End of recursion.
        typedef typename TypeAt<Transitions, 0>::Result FirstTransition;
        typedef typename FirstTransition::ToType::ObjectType ToType;
        typedef typename FirstTransition::CreationPolicyType CreationPolicy;

        // TODO: Mustn't be a choice transition
        //CompileTimeError < is_same<ToType, EmptyState<typename CreationPolicy::ObjectType>>().value >();

        bool hasSameFromState = entryState->template typeOf<ToType>();

        bool conditionMet = FirstTransition::N == N && hasSameFromState;
        if (conditionMet) {
          ToType* state = static_cast<ToType*>(entryState);
          state->template _entry<N>();
          state->template _doit<N>();
        }
        return;
      }
    };

    template<typename T, uint8_t N, int Index>
    struct Initializer {
      static DispatchResult<StateType> init() {
        typedef typename TypeAt<T, Index>::Result CurrentTransition;
        if (CurrentTransition::E && CurrentTransition::N == N) {
          auto result = CurrentTransition().dispatch(0);
          if (result.consumed) {
            return result;
          }
        }
        // Recursion
        return Initializer < T, N, Index - 1 >::init();
      }
    };
    // Specialization
    template<typename T, uint8_t N>
    struct Initializer<T, N, 0> {
      static DispatchResult<StateType> init() {
        // End of recursion.
        typedef typename TypeAt<T, 0>::Result FirstTransition;
        if (FirstTransition::E && FirstTransition::N == N) {
          auto result = FirstTransition().dispatch(0);
          if (result.consumed) {
            return result;
          }
        }
        return Initialtransition().dispatch(0);
      }
    };

    template<typename T, uint8_t N, int Index>
    struct Finisher {
      static DispatchResult<StateType> end(StateType* activeState) {
        typedef typename TypeAt<T, Index>::Result CurrentTransition;
        if (CurrentTransition::X && CurrentTransition::N == N) {
          auto result = CurrentTransition().dispatch(activeState);
          if (result.consumed) {
            return result;
          }
        }
        // Recursion
        return Finisher < T, N, Index - 1 >::end(activeState);
      }
    };
    // Specialization
    template<typename T, uint8_t N>
    struct Finisher<T, N, 0> {
      static DispatchResult<StateType> end(StateType* activeState) {
        // End of recursion.
        typedef typename TypeAt<T, 0>::Result FirstTransition;
        if (FirstTransition::X && FirstTransition::N == N) {
          auto result = FirstTransition().dispatch(activeState);
          if (result.consumed) {
            return result;
          }
        }
        return Endtransition().dispatch(activeState);
      }
    };

};
}
