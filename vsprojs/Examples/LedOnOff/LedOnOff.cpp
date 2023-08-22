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

#define BSP_Execute(x) std::cout << #x << std::endl

#include "../../../src/tsm.h"
#include <iostream>

using namespace tsmlib;
using namespace std;

using StateType = State<MemoryAddressComparator, true>;
using StateTypeCreationPolicyType = SingletonCreator<StateType>;

namespace Trigger {
  struct Timeout;
}

class LedOn : public BasicState<LedOn, StateType>, public SingletonCreator<LedOn> {
  friend class BasicState<LedOn, StateType>;
  void entry() {
    BSP_Execute(digitalWrite(LED_BUILTIN, HIGH));
  }
  void exit() {}
  template<class Event>
  void doit() {}
};

class LedOff : public BasicState<LedOff, StateType>, public SingletonCreator<LedOff> {
  friend class BasicState<LedOff, StateType>;
  void entry() {
    BSP_Execute(digitalWrite(LED_BUILTIN, LOW));
    BSP_Execute(Serial.println(freeMemory()));
  }
  void exit() {}
  template<class event>
  void doit() {}
};

using ToOnFromOff = Transition<Trigger::Timeout, LedOn, LedOff, StateTypeCreationPolicyType, NoGuard, NoAction>;
using ToOffFromOn = Transition<Trigger::Timeout, LedOff, LedOn, StateTypeCreationPolicyType, NoGuard, NoAction>;

using Transitions = 
  Typelist<ToOnFromOff,
  Typelist<ToOffFromOn,
  NullType>>;

using InitTransition = InitialTransition<LedOff, StateTypeCreationPolicyType, NoAction>;
using Sm = Statemachine<Transitions, InitTransition>;

Sm statemachine;

void setup() {
  BSP_Execute(Serial.begin(9600));
  BSP_Execute(pinMode(LED_BUILTIN, OUTPUT));
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Trigger::Timeout>();
  BSP_Execute(delay(1000));
  statemachine.dispatch<Trigger::Timeout>();
  BSP_Execute(delay(1000));
}

int main()
{
  setup();
  loop();
}
