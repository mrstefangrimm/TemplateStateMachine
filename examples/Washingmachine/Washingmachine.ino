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
#include "WashingMachine.h"

int ledState = LOW;
void blink() {
  BSP_Execute(digitalWrite(LED_BUILTIN, ledState));
  ledState = ledState == LOW ? HIGH : LOW;
}