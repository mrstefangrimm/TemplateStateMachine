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
    STATE* trigger() { return 0; }
    static NullStatemachine<STATE>* Instance;
  };
  template<typename T> NullStatemachine<T>* NullStatemachine<T>::Instance = 0;

  template<typename STATE, typename TRANSITIONS, typename INNERSM, typename INITIALTRANSITION, typename FINALTRANSITION>
  class Statemachine {
  public:

    Statemachine() {
      INITIALTRANSITION initialTransition;
      _activeState = initialTransition.trigger(0);
    }

    template<uint8_t T>
    STATE* trigger() {

      if (_activeState == 0) return _activeState;
      
      // Final transition. Check guard and terminate.
      //if (!is_same<NullFinalTransition<STATE>, FINALTRANSITION>().value) {

      //  typename FINALTRANSITION::GuardType guard;
      //  if (guard.check()) {

      //    typedef typename TypeAt<STATES, _activeStateIdx>::Result ActiveState;
      //    ActiveState activeState;
      //    activeState.exit();

      //    _activeState = FINALTRANSITION().Trigger(_activeState);
      //    
      //    if (_activeState == 0) return _activeState;
      //  }
      //}

      //  //TINNERSM* innerStateMachine = TINNERSM::Instance;
      //  //innerStateMachine->trigger();

      typedef typename TypeAt<TRANSITIONS, T>::Result Transistion;
      
      STATE* state = Transistion().trigger(_activeState);
      //_activeStateIdx = IndexOf<STATES, typename Transistion::ToType>::Result;
      _activeState = state;
      return _activeState;
    }
  private:
    STATE* _activeState;
    //uint8_t _activeStateIdx;
    //static Statemachine<STATE, TRANSITIONS, INNERSM, INITIALSTATEFAC>* Instance;
  };
  //template<typename TSTATE, typename TTRANSITIONS, typename TINNERSM, typename TINITSTATEFAC> 
  //Statemachine<TSTATE, TTRANSITIONS, TINNERSM, TINITSTATEFAC>* Statemachine<TSTATE, TTRANSITIONS, TINNERSM, TINITSTATEFAC>::Instance = new Statemachine<TSTATE, TTRANSITIONS, TINNERSM, TINITSTATEFAC>;

}
