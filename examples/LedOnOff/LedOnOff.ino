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
#define IAMARDUINO 1

#define BSP_Execute(x) x

#include "tsm.h"
#include "FreeMemory.h"

using namespace tsmlib;

typedef State<MemoryAddressComparator, true> StateType;
typedef SingletonCreator<StateType> StateTypeCreationPolicyType;

enum Trigger {
  TIMEOUT,
};

class LedOn : public BasicState<LedOn, StateType>, public SingletonCreator<LedOn> {
  friend class BasicState<LedOn, StateType>;
  void entry() {
    BSP_Execute(digitalWrite(LED_BUILTIN, HIGH));
  }
  void exit() {}
  template<uint8_t N>
  void doit() {}
};

class LedOff : public BasicState<LedOff, StateType>, public SingletonCreator<LedOff> {
  friend class BasicState<LedOff, StateType>;
  void entry() {
    BSP_Execute(digitalWrite(LED_BUILTIN, LOW));
    BSP_Execute(Serial.println(freeMemory()));
  }
  void exit() {}
  template<uint8_t N>
  void doit() {}
};

typedef Transition<Trigger::TIMEOUT, LedOn, LedOff, StateTypeCreationPolicyType, NoGuard, NoAction> ToOnFromOff;
typedef Transition<Trigger::TIMEOUT, LedOff, LedOn, StateTypeCreationPolicyType, NoGuard, NoAction> ToOffFromOn;

typedef Typelist<ToOnFromOff,
                 Typelist<ToOffFromOn,
                          NullType>>
  TransitionList;

typedef InitialTransition<LedOff, StateTypeCreationPolicyType, NoAction> InitTransition;
typedef Statemachine<
  TransitionList,
  InitTransition >
  Sm;

Sm statemachine;

void setup() {
  BSP_Execute(Serial.begin(9600));
  BSP_Execute(pinMode(LED_BUILTIN, OUTPUT));
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Trigger::TIMEOUT>();
  BSP_Execute(delay(1000));
  statemachine.dispatch<Trigger::TIMEOUT>();
  BSP_Execute(delay(1000));
}
