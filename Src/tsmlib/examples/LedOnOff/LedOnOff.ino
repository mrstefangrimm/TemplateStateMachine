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

class LedOn : public SimpleState<LedOn, StateType>, public SingletonCreator<LedOn> {
    friend class SimpleState<LedOn, StateType>;
    void entry_() {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    void exit_() { }
    template<uint8_t N>
    void doit_() { }
};

class LedOff : public SimpleState<LedOff, StateType>, public SingletonCreator<LedOff> {
    friend class SimpleState<LedOff, StateType>;
    void entry_() {
      digitalWrite(LED_BUILTIN, LOW);
    }
    void exit_() { }
    template<uint8_t N>
    void doit_() { }
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
