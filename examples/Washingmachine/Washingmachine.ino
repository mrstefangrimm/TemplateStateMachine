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
#define IAMARDUINO 1

#define BSP_Execute(x) x

#include "tsm.h"
using namespace tsmlib;

int ledState = LOW;
void blink() {
 BSP_Execute(digitalWrite(LED_BUILTIN, ledState);)
 ledState = ledState == LOW ? HIGH : LOW;
}

typedef State<MemoryAddressStateComperator<true>, true> StateType;
typedef SingletonCreator<StateType> StateTypeCreationPolicyType;

enum Triggers {
  TIMEOUT,
};

struct Loading : public SimpleState<Loading, StateType>, public SingletonCreator<Loading> {
  void entry_() {
    BSP_Execute(digitalWrite(LED_BUILTIN, LOW);)
      BSP_Execute(Serial.println(F("Loading"));)
      BSP_Execute(Serial.println(F("  Door open."));)
  }
  void exit_() {
    BSP_Execute(Serial.println(F("  Door closed."));)
  }
  template<uint8_t N>
  void doit_() { }
};

struct Washing : public SimpleState<Washing, StateType>, public SingletonCreator<Washing> {
  void entry_() {
    BSP_Execute(Serial.println(F("  Washing"));)
    counter_ = 0;
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() {
  }

  uint8_t counter_ = 0;
  const uint8_t washingLength_ = 50;
};

struct Rinsing : public SimpleState<Rinsing, StateType>, public SingletonCreator<Rinsing> {
  void entry_() {
    BSP_Execute(Serial.println(F("  Rinsing"));)
    counter_ = 0;
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }

  uint8_t counter_ = 0;
  const uint8_t rinsingLength_ = 30;
};

struct Spinning : public SimpleState<Spinning, StateType>, public SingletonCreator<Spinning> {
  void entry_() {
    BSP_Execute(Serial.println(F("  Spinning"));)
    counter_ = 0;
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }

  uint8_t counter_ = 0;
  const uint8_t spinningLength_ = 40;
};

struct IsWashingAction {
  template<typename T>
  void perform(T* activeState) {
    auto washingState = static_cast<Washing*>(activeState);
    washingState->counter_++;
    if (washingState->counter_ % 4 == 0) {
      BSP_Execute(blink();)
    }
  }
};
struct IsWashingDone {
  template<typename T> bool check(T* activeState) {
    auto washingState = static_cast<Washing*>(activeState);
    return washingState->counter_ > washingState->washingLength_;
  }
};

struct IsRinsingAction {
  template<typename T>
  void perform(T* activeState) {
    auto rinnsingState = static_cast<Rinsing*>(activeState);
    rinnsingState->counter_++;
    if (rinnsingState->counter_ % 2 == 0) {
      BSP_Execute(blink();)
    }
  }
};
struct IsRinsingDone {
  template<typename T> bool check(T* activeState) {
    auto rinnsingState = static_cast<Rinsing*>(activeState);
    return rinnsingState->counter_ > rinnsingState->rinsingLength_;
  }
};

struct IsSpinningAction {
  template<typename T>
  void perform(T* activeState) {
    auto spinningState = static_cast<Spinning*>(activeState);
    spinningState->counter_++;
    BSP_Execute(blink();)
  }
};
struct IsSpinningDone {
  template<typename T> bool check(T* activeState) {
    auto spinningState = static_cast<Spinning*>(activeState);
    return spinningState->counter_ > spinningState->spinningLength_;
  }
};

typedef Transition<Triggers::TIMEOUT, struct Running, Loading, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToRunningFromLoading;
typedef Declaration<Triggers::TIMEOUT, struct Running, StateTypeCreationPolicyType> TimeoutDeclaration;
typedef
Typelist<ToRunningFromLoading,
  Typelist<TimeoutDeclaration,
  NullType>> TransitionList;

typedef Transition<Triggers::TIMEOUT, Rinsing, Washing, StateTypeCreationPolicyType, IsWashingDone, IsWashingAction> ToRinsingFromWashing;
typedef Transition<Triggers::TIMEOUT, Spinning, Rinsing, StateTypeCreationPolicyType, IsRinsingDone, IsRinsingAction> ToSpinningFromRinsing;
typedef ExitTransition<Triggers::TIMEOUT, Loading, Spinning, StateTypeCreationPolicyType, IsSpinningDone, IsSpinningAction> ToLoadingFromSpinning;
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

struct Running : public SubstatesHolderState<Running, StateType, RunningSm>, public SingletonCreator<Running> {
  void entry_() {
    BSP_Execute(Serial.println(F("Running"));)
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }
};

Sm statemachine;

void setup() {
  BSP_Execute(Serial.begin(9600);)
  BSP_Execute(pinMode(LED_BUILTIN, OUTPUT);)
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Triggers::TIMEOUT>();
  BSP_Execute(delay(100);)
}
