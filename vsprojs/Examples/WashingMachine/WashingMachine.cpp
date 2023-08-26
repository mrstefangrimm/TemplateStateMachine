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

#include <iostream>
#include "../../../src/tsm.h"
#include "../../../examples/Washingmachine/WashingMachine.h"

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
