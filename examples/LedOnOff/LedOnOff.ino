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
#define DISABLE_NESTED_STATES
#define IAMARDUINO 1
#include "tsm.h"

#include "FreeMemory.h"

using namespace tsmlib;

typedef State<MemoryAddressStateComperator<true>, true> StateType;
typedef SingletonCreator<StateType> StateTypeCreationPolicyType;

enum Triggers {
  TIMEOUT,
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
      Serial.println(freeMemory());
    }
    void exit_() { }
    template<uint8_t N>
    void doit_() { }
};

typedef Transition<Triggers::TIMEOUT, LedOn, LedOff, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOnFromOff;
typedef Transition<Triggers::TIMEOUT, LedOff, LedOn, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOffFromOn;

typedef
Typelist<ToOnFromOff,
  Typelist<ToOffFromOn,
  NullType>> TransitionList;

typedef InitialTransition<LedOff, StateTypeCreationPolicyType, EmptyAction> InitTransition;
typedef Statemachine <
  TransitionList,
  InitTransition,
  NullEndTransition<StateTypeCreationPolicyType>> Sm;

Sm statemachine;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Triggers::TIMEOUT>();
  delay(1000);
  statemachine.dispatch<Triggers::TIMEOUT>();
  delay(1000);
}
