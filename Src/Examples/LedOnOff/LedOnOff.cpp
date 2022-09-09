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

#define DISABLENESTEDSTATES

#include "tsmlib\tsm.h"

#include <iostream>

using namespace tsmlib;
using namespace std;

typedef State<MemoryAddressStateComperator<true>, true> StateType;

enum Triggers {
  On,
  Off,
};

class LedOn : public SimpleState<LedOn, StateType>, public SingletonCreator<LedOn> {
  friend class SimpleState<LedOn, StateType>;
  void entry_() {
    cout << "digitalWrite(LED_BUILTIN, HIGH);" << endl;
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }
};

class LedOff : public SimpleState<LedOff, StateType>, public SingletonCreator<LedOff> {
  friend class SimpleState<LedOff, StateType>;
  void entry_() {
    cout << "digitalWrite(LED_BUILTIN, LOW)" << endl;
  }
  void exit_() { }
  template<uint8_t N>
  void doit_() { }
};

typedef Transition<Triggers::On, StateType, LedOn, LedOff, OkGuard, EmptyAction> ToOnFromOff;
typedef Transition<Triggers::Off, StateType, LedOff, LedOn, OkGuard, EmptyAction> ToOffFromOn;

typedef
Typelist<ToOnFromOff,
  Typelist<ToOffFromOn,
  NullType>> TransitionList;

typedef InitialTransition<StateType, LedOff, EmptyAction> InitTransition;
typedef Statemachine <
  StateType,
  TransitionList,
  InitTransition,
  NullEndTransition<StateType>> SM;

SM statemachine;

void setup() {
  cout << "pinMode(LED_BUILTIN, OUTPUT);" << endl;
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Triggers::On>();
  //delay(1000);
  statemachine.dispatch<Triggers::Off>();
  //delay(1000);
}

int main()
{
  setup();
  loop();
}
