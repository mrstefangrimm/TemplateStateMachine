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
#include "tsm.h"

using namespace tsmlib;

typedef State<MemoryAddressStateComperator<true>, true> StateType;

enum Triggers {
  On,
  Off,
};

struct LedOn : StateType, SingletonCreator<LedOn> {
  void entry() {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  void exit() { }
  void doit() { }
};

struct LedOff : StateType, SingletonCreator<LedOff> {
  void entry() {
    digitalWrite(LED_BUILTIN, LOW);
  }
  void exit() { }
  void doit() { }
};

typedef Transition<Triggers::On, StateType, LedOn, LedOff, EmptyGuard, EmptyAction> ToOnFromOff_t;
typedef Transition<Triggers::Off, StateType, LedOff, LedOn, EmptyGuard, EmptyAction> ToOffFromOn_t;

typedef
Typelist<ToOnFromOff_t,
         Typelist<ToOffFromOn_t,
         NullType>> TransitionList;

typedef InitialTransition<StateType, LedOff, EmptyAction> InitTransition;
typedef Statemachine <
StateType,
TransitionList,
NullStatemachine<StateType>,
InitTransition,
NullFinalTransition<StateType >> SM;

SM statemachine;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  statemachine.begin();
}

void loop() {
  statemachine.trigger<Triggers::On>();
  delay(1000);
  statemachine.trigger<Triggers::Off>();
  delay(1000);
}
