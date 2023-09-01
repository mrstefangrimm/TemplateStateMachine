#pragma once
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

using namespace tsmlib;

namespace Trigger {
struct Timeout {};
}

using StatePolicy = State<MemoryAddressComparator, true>;

class LedOn : public BasicState<LedOn, StatePolicy, true>, public SingletonCreator<LedOn> {
  friend class BasicState<LedOn, StatePolicy, true>;
  template<class Event>
  void entry(const Event&) {
    BSP_Execute(digitalWrite(LED_BUILTIN, HIGH));
  }
};

class LedOff : public BasicState<LedOff, StatePolicy, true>, public SingletonCreator<LedOff> {
  friend class BasicState<LedOff, StatePolicy, true>;
  template<class Event>
  void entry(const Event& ev) {
    BSP_Execute(digitalWrite(LED_BUILTIN, LOW));
  }
};

using ToOnFromOff = Transition<Trigger::Timeout, LedOn, LedOff, NoGuard, NoAction>;
using ToOffFromOn = Transition<Trigger::Timeout, LedOff, LedOn, NoGuard, NoAction>;

using Transitions =
  Typelist<ToOnFromOff,
           Typelist<ToOffFromOn,
                    NullType>>;

using InitTransition = InitialTransition<LedOff, NoAction>;
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
}
