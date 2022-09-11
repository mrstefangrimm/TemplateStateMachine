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
//#define IAMWINDOWS 1

#define IAMARDUINO 1
// Defines unint8_t which does not require an include on Arduino.
#include <stdint.h>

#define BSP_Execute(x) std::cout << #x << std::endl;

#include "..\..\..\src\tsm.h"
#include <iostream>

using namespace tsmlib;
using namespace std;

typedef State<MemoryAddressStateComperator<true>, true> StateType;
typedef SingletonCreator<StateType> StateTypeCreationPolicyType;

enum Triggers {
  TIMEOUT,
  LEAVING_TIMEOUT,
};

class Loading : public SimpleState<Loading, StateType>, public SingletonCreator<Loading> {
  friend class SimpleState<Loading, StateType>;
  void entry_() {
    BSP_Execute(digitalWrite(LED_L, LOW);)
    BSP_Execute(Serial.println(F("Loading"));)
    BSP_Execute(Serial.println(F("  Door open."));)
  }
  void exit_() {
    BSP_Execute(Serial.println(F("  Door closed."));)
  }
  template<uint8_t N>
  void doit_() { }
};

class Washing : public SimpleState<Washing, StateType>, public SingletonCreator<Washing> {
  friend class SimpleState<Washing, StateType>;
  void entry_() {
    BSP_Execute(Serial.println(F("  Washing"));)
    counter_ = 0;
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() {
  }

  friend struct IsWashingAction;
  friend struct IsWashingDone;
  uint8_t counter_ = 0;
};

class Rinsing : public SimpleState<Rinsing, StateType>, public SingletonCreator<Rinsing> {
  friend class SimpleState<Rinsing, StateType>;
  void entry_() {
    BSP_Execute(Serial.println(F("  Rinsing"));)
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }

  friend struct IsRinsingAction;
  friend struct IsRinsingDone;
  uint8_t counter_ = 0;
};

class Spinning : public SimpleState<Spinning, StateType>, public SingletonCreator<Spinning> {
  friend class SimpleState<Spinning, StateType>;
  void entry_() {
    BSP_Execute(Serial.println(F("  Spinning"));)
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }

  friend struct IsSpinningAction;
  friend struct IsSpinningDone;
  uint8_t counter_ = 0;
};

struct IsWashingAction {
  template<typename T>
  void perform(T* activeState) {
    static_cast<Washing*>(activeState)->counter_++;
    if (static_cast<Washing*>(activeState)->counter_ % 4 == 0) {
      BSP_Execute(Washingmachine_blink(me);)
    }
  }
};
struct IsWashingDone {
  template<typename T> bool check(T* activeState) { return static_cast<Washing*>(activeState)->counter_ > 0; }
};

struct IsRinsingAction {
  template<typename T>
  void perform(T* activeState) {
    static_cast<Rinsing*>(activeState)->counter_++;
    if (static_cast<Rinsing*>(activeState)->counter_ % 2 == 0) {
      BSP_Execute(Washingmachine_blink(me);)
    }
  }
};
struct IsRinsingDone {
  template<typename T> bool check(T* activeState) { return static_cast<Rinsing*>(activeState)->counter_ > 0; }
};

struct IsSpinningAction {
  template<typename T>
  void perform(T* activeState) {
    static_cast<Spinning*>(activeState)->counter_++;
    if (static_cast<Spinning*>(activeState)->counter_ % 2 == 0) {
      BSP_Execute(Washingmachine_blink(me);)
    }
  }
};
struct IsSpinningDone {
  template<typename T> bool check(T* activeState) {
    int cnt = static_cast<Spinning*>(activeState)->counter_;
    if (cnt > 0) {
      return true;
    }
    return false;
  }
};

//typedef ::impl::TransitionBase<Triggers::TIMEOUT, Loading, Spinning, StateTypeCreationPolicyType, IsSpinningDone, IsSpinningAction, true> ToLoadingFromSpinning;
//struct CanExit {
//  template<typename T> bool check(T* activeState) {
//    return false;
//  }
//};

typedef Transition<Triggers::TIMEOUT, class Running, Loading, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToRunningFromLoading;
typedef Transition<Triggers::TIMEOUT, Rinsing, Washing, StateTypeCreationPolicyType, IsWashingDone, IsWashingAction> ToRinsingFromWashing;
typedef Transition<Triggers::TIMEOUT, Spinning, Rinsing, StateTypeCreationPolicyType, IsRinsingDone, IsRinsingAction> ToSpinningFromRinsing;
//typedef Transition<Triggers::TIMEOUT, Loading, Spinning, StateTypeCreationPolicyType, IsSpinningDone, IsSpinningAction> ToLoadingFromSpinning;
typedef Transition<Triggers::LEAVING_TIMEOUT, Loading, Spinning, StateTypeCreationPolicyType, IsSpinningDone, IsSpinningAction> ToLoadingFromSpinning;

typedef
Typelist<ToRunningFromLoading,
  Typelist<typename Declaration<Triggers::TIMEOUT, Running, StateTypeCreationPolicyType>,
  //Typelist<typename ExitDeclaration2<Triggers::TIMEOUT, Loading, Running, StateTypeCreationPolicyType, CanExit>,
  Typelist<typename ExitDeclaration<Triggers::LEAVING_TIMEOUT, Loading, Running, StateTypeCreationPolicyType>,
  NullType>>> TransitionList;

typedef
Typelist<ToRunningFromLoading,
  Typelist< ToRinsingFromWashing,
  Typelist< ToSpinningFromRinsing,
  Typelist< ToLoadingFromSpinning,
  NullType>>>> RunningTransitionList;

typedef InitialTransition<Loading, StateTypeCreationPolicyType, EmptyAction> InitTransition;
typedef Statemachine <
  TransitionList,
  InitTransition,
  NullEndTransition<StateTypeCreationPolicyType>> Sm;

typedef InitialTransition<Washing, StateTypeCreationPolicyType, EmptyAction> RunningInitTransition;
typedef Statemachine <
  RunningTransitionList,
  RunningInitTransition,
  NullEndTransition<StateTypeCreationPolicyType>> RunningSm;

class Running : public SubstatesHolderState<Running, StateType, RunningSm>, public SingletonCreator<Running> {
  friend class SubstatesHolderState<Running, StateType, RunningSm>;
  void entry_() {
    BSP_Execute(Serial.println(F("Running"));)
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }
};

struct ExitRunning {
  template<typename T> bool check(T* activeState) {
    Running* state = static_cast<class Running*>(activeState);
    return true;
  }
};

Sm statemachine;

void setup() {
  BSP_Execute(cout << "pinMode(LED_BUILTIN, OUTPUT);" << endl;)
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Triggers::TIMEOUT>();
  BSP_Execute(delay(100);)
}

int main()
{
  setup();
  // Loading
  statemachine.dispatch<Triggers::TIMEOUT>();
  // Running/Washing
  statemachine.dispatch<Triggers::TIMEOUT>();
  // Running/Rinsing
  statemachine.dispatch<Triggers::TIMEOUT>();
  // Running/Spinning
  //statemachine.dispatch<Triggers::TIMEOUT>();
  statemachine.dispatch<Triggers::LEAVING_TIMEOUT>();
  // Loading
  statemachine.dispatch<Triggers::TIMEOUT>();
  // Running/Washing
  statemachine.dispatch<Triggers::TIMEOUT>();
  // Running/Rinsing
  statemachine.dispatch<Triggers::TIMEOUT>();
  // Running/Spinning
  statemachine.dispatch<Triggers::LEAVING_TIMEOUT>();

  while (true) {
    loop();
  }
}
