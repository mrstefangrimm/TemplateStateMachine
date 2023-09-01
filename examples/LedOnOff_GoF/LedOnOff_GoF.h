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

enum Trigger {
  TIMEOUT,
};

class LedState {
public:
  virtual void timeout(class Statemachine* statemachine) = 0;
};

class LedOn : public LedState {
  void timeout(class Statemachine* statemachine) override;
};

class LedOff : public LedState {
  void timeout(class Statemachine* statemachine) override;
};

class Statemachine {
public:
  void begin() {
    activeState_ = ledOff;
  }
  void dispatchTimeout() {
    activeState_->timeout(this);
  }

protected:
  friend class LedState;
  friend class LedOff;
  friend class LedOn;

  void changeLedState(LedState* state);

  LedState* activeState_;
  static LedState* ledOn;
  static LedState* ledOff;
};
LedState* Statemachine::ledOn = new LedOn;
LedState* Statemachine::ledOff = new LedOff;

void LedOn::timeout(class Statemachine* statemachine) {
  BSP_Execute(digitalWrite(LED_BUILTIN, HIGH));
    statemachine->changeLedState(Statemachine::ledOff);
}

void LedOff::timeout(class Statemachine* statemachine) {
  BSP_Execute(digitalWrite(LED_BUILTIN, LOW));
  statemachine->changeLedState(Statemachine::ledOn);
}

void Statemachine::changeLedState(LedState* state) {
  activeState_ = state;
}

Statemachine statemachine;

void setup() {
  BSP_Execute(Serial.begin(9600));
    BSP_Execute(pinMode(LED_BUILTIN, OUTPUT));
    statemachine.begin();
}

void loop() {
  statemachine.dispatchTimeout();
  BSP_Execute(delay(1000));
}
