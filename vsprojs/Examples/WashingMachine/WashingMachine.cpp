/*
  Copyright 2022-2023 Stefan Grimm

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
#define IAMWORKSTATION 1

#define BSP_Execute(x) std::cout << #x << std::endl;

#include "..\..\..\src\tsm.h"
#include <iostream>

using namespace tsmlib;
using namespace std;

typedef State<MemoryAddressComparator, true> StateType;
typedef SingletonCreator<StateType> StateTypeCreationPolicyType;

namespace Trigger {
  struct Timeout;
}

struct Loading;
struct Running;
struct Washing;
struct Rinsing;
struct Spinning;

struct IsWashingAction {
  template<class T>
  void perform(T* activeState) {
    auto washingState = static_cast<Washing*>(activeState);
    washingState->counter_++;
    if (washingState->counter_ % 4 == 0) {
      BSP_Execute(blink());
    }
  }
};

struct IsWashingDone {
  template<class T>
  bool eval(T* activeState) {
    auto washingState = static_cast<Washing*>(activeState);
    return washingState->counter_ > washingState->washingLength_;
  }
};

struct IsRinsingAction {
  template<class T>
  void perform(T* activeState) {
    auto rinnsingState = static_cast<Rinsing*>(activeState);
    rinnsingState->counter_++;
    if (rinnsingState->counter_ % 2 == 0) {
      BSP_Execute(blink());
    }
  }
};

struct IsRinsingDone {
  template<class T>
  bool eval(T* activeState) {
    auto rinnsingState = static_cast<Rinsing*>(activeState);
    return rinnsingState->counter_ > rinnsingState->rinsingLength_;
  }
};

struct IsSpinningAction {
  template<class T>
  void perform(T* activeState) {
    auto spinningState = static_cast<Spinning*>(activeState);
    spinningState->counter_++;
    BSP_Execute(blink());
  }
};

struct IsSpinningDone {
  template<class T>
  bool eval(T* activeState) {
    auto spinningState = static_cast<Spinning*>(activeState);
    return spinningState->counter_ > spinningState->spinningLength_;
  }
};

typedef ChoiceTransition<Trigger::Timeout, Rinsing, Washing, Washing, StateTypeCreationPolicyType, IsWashingDone, IsWashingAction> ToRinsingFromWashing;
typedef ChoiceTransition<Trigger::Timeout, Spinning, Rinsing, Rinsing, StateTypeCreationPolicyType, IsRinsingDone, IsRinsingAction> ToSpinningFromRinsing;
typedef ChoiceExitTransition<Trigger::Timeout, Loading, Spinning, Spinning, StateTypeCreationPolicyType, IsSpinningDone, IsSpinningAction> ToLoadingFromSpinning;
typedef Typelist< ToRinsingFromWashing,
  Typelist< ToSpinningFromRinsing,
  Typelist< ToLoadingFromSpinning,
  NullType>>>
  RunningTransitionList;

typedef InitialTransition<Washing, StateTypeCreationPolicyType, NoAction> RunningInitTransition;
typedef Statemachine<
  RunningTransitionList,
  RunningInitTransition>
  RunningSm;

typedef Transition<Trigger::Timeout, Running, Loading, StateTypeCreationPolicyType, NoGuard, NoAction> ToRunningFromLoading;
typedef ExitDeclaration<Trigger::Timeout, Loading, Running, StateTypeCreationPolicyType> TimeoutDeclaration;
typedef Typelist<ToRunningFromLoading,
  Typelist<TimeoutDeclaration,
  NullType>>
  WashingmachineTransitionList;

typedef InitialTransition<Loading, StateTypeCreationPolicyType, NoAction> InitTransition;
typedef Statemachine<
  WashingmachineTransitionList,
  InitTransition>
  WashingmachineSm;

struct Loading : public BasicState<Loading, StateType>, public SingletonCreator<Loading> {
  void entry() {
    BSP_Execute(digitalWrite(LED_BUILTIN, LOW));
    BSP_Execute(Serial.println(F("Loading")));
    BSP_Execute(Serial.println(F("  Door open.")));
  }
  void exit() {
    BSP_Execute(Serial.println(F("  Door closed.")));
  }
  template<class Event>
  void doit() {}
};

struct Running : public SubstatesHolderState<Running, StateType, RunningSm>, public SingletonCreator<Running> {
  void entry() {
    BSP_Execute(Serial.println(F("Running")));
  }
  void exit() {}
  template<class Event>
  void doit() {}
};

struct Washing : public BasicState<Washing, StateType>, public SingletonCreator<Washing> {
  void entry() {
    BSP_Execute(Serial.println(F("  Washing")));
    counter_ = 0;
  }
  void exit() {}
  template<class Event>
  void doit() {
  }

  uint8_t counter_ = 0;
  const uint8_t washingLength_ = 50;
};

struct Rinsing : public BasicState<Rinsing, StateType>, public SingletonCreator<Rinsing> {
  void entry() {
    BSP_Execute(Serial.println(F("  Rinsing")));
    counter_ = 0;
  }
  void exit() {}
  template<class Event>
  void doit() {}

  uint8_t counter_ = 0;
  const uint8_t rinsingLength_ = 30;
};

struct Spinning : public BasicState<Spinning, StateType>, public SingletonCreator<Spinning> {
  void entry() {
    BSP_Execute(Serial.println(F("  Spinning")));
    counter_ = 0;
  }
  void exit() {}
  template<class Event>
  void doit() {}

  uint8_t counter_ = 0;
  const uint8_t spinningLength_ = 40;
};

WashingmachineSm statemachine;

void setup() {
  BSP_Execute(Serial.begin(9600));
  BSP_Execute(pinMode(LED_BUILTIN, OUTPUT));
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Trigger::Timeout>();
  BSP_Execute(delay(100));
}

int main()
{
  setup();
  // Loading
  statemachine.dispatch<Trigger::Timeout>();
  // Running/Washing
  statemachine.dispatch<Trigger::Timeout>();
  // Running/Rinsing
  statemachine.dispatch<Trigger::Timeout>();
  // Running/Spinning
  statemachine.dispatch<Trigger::Timeout>();
  // Loading
  statemachine.dispatch<Trigger::Timeout>();
  // Running/Washing
  statemachine.dispatch<Trigger::Timeout>();
  // Running/Rinsing
  statemachine.dispatch<Trigger::Timeout>();
  // Running/Spinning
  statemachine.dispatch<Trigger::Timeout>();

  while (true) {
    loop();
  }
}
