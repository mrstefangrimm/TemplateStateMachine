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

using StateType = State<MemoryAddressComparator, true>;
using StateTypeCreationPolicyType = SingletonCreator<StateType>;

namespace Trigger {
struct Timeout {};
}

struct Loading;
struct Running;
struct Washing;
struct Rinsing;
struct Spinning;

struct IsWashingAction {
  template<class StateType, class EventType>
  void perform(StateType& activeState, const EventType&) {
    activeState.counter_++;
    if (activeState.counter_ % 4 == 0) {
      BSP_Execute(blink());
    }
  }
};

struct IsWashingDone {
  template<class StateType, class EventType>
  bool eval(const StateType& activeState, const EventType&) {
    return activeState.counter_ > activeState.washingLength_;
  }
};

struct IsRinsingAction {
  template<class StateType, class EventType>
  void perform(StateType& activeState, const EventType&) {
    activeState.counter_++;
    if (activeState.counter_ % 2 == 0) {
      BSP_Execute(blink());
    }
  }
};

struct IsRinsingDone {
  template<class StateType, class EventType>
  bool eval(const StateType& activeState, const EventType&) {
    return activeState.counter_ > activeState.rinsingLength_;
  }
};

struct IsSpinningAction {
  template<class StateType, class EventType>
  void perform(StateType& activeState, const EventType&) {
    activeState.counter_++;
    BSP_Execute(blink());
  }
};

struct IsSpinningDone {
  template<class StateType, class EventType>
  bool eval(const StateType& activeState, const EventType&) {
    return activeState.counter_ > activeState.spinningLength_;
  }
};

using ToRinsingFromWashing = ChoiceTransition<Trigger::Timeout, Rinsing, Washing, Washing, StateTypeCreationPolicyType, IsWashingDone, IsWashingAction>;
using ToSpinningFromRinsing = ChoiceTransition<Trigger::Timeout, Spinning, Rinsing, Rinsing, StateTypeCreationPolicyType, IsRinsingDone, IsRinsingAction>;
using ToLoadingFromSpinning = ChoiceExitTransition<Trigger::Timeout, Loading, Spinning, Spinning, StateTypeCreationPolicyType, IsSpinningDone, IsSpinningAction>;
using RunningTransitionList = Typelist<ToRinsingFromWashing, Typelist<ToSpinningFromRinsing, Typelist<ToLoadingFromSpinning, NullType>>>;
using RunningInitTransition = InitialTransition<Washing, StateTypeCreationPolicyType, NoAction>;
using RunningSm = Statemachine<RunningTransitionList, RunningInitTransition>;

using ToRunningFromLoading = Transition<Trigger::Timeout, Running, Loading, StateTypeCreationPolicyType, NoGuard, NoAction>;
using TimeoutDeclaration = ExitDeclaration<Trigger::Timeout, Loading, Running, StateTypeCreationPolicyType>;
using WashingmachineTransitionList =
  Typelist<ToRunningFromLoading,
           Typelist<TimeoutDeclaration,
                    NullType>>;

using InitTransition = InitialTransition<Loading, StateTypeCreationPolicyType, NoAction>;
using WashingmachineSm = Statemachine<WashingmachineTransitionList, InitTransition>;

struct Loading : public BasicState<Loading, StateType>, public SingletonCreator<Loading> {
  template<class Event> void entry(const Event& ev) {
    BSP_Execute(digitalWrite(LED_BUILTIN, LOW));
    BSP_Execute(Serial.println(F("Loading")));
    BSP_Execute(Serial.println(F("  Door open.")));
  }
  template<class Event> void exit(const Event&) {
    BSP_Execute(Serial.println(F("  Door closed.")));
  }
  template<class Event> void doit(const Event&) {}
};

struct Running : public SubstatesHolderState<Running, StateType, RunningSm>, public SingletonCreator<Running> {
  template<class Event> void entry(const Event&) {
    BSP_Execute(Serial.println(F("Running")));
  }
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) {}
};

struct Washing : public BasicState<Washing, StateType>, public SingletonCreator<Washing> {
  template<class Event> void entry(const Event&) {
    BSP_Execute(Serial.println(F("  Washing")));
    counter_ = 0;
  }
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) {}

  uint8_t counter_ = 0;
  const uint8_t washingLength_ = 50;
};

struct Rinsing : public BasicState<Rinsing, StateType>, public SingletonCreator<Rinsing> {
  template<class Event> void entry(const Event&) {
    BSP_Execute(Serial.println(F("  Rinsing")));
    counter_ = 0;
  }
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) {}

  uint8_t counter_ = 0;
  const uint8_t rinsingLength_ = 30;
};

struct Spinning : public BasicState<Spinning, StateType>, public SingletonCreator<Spinning> {
  template<class Event> void entry(const Event&) {
    BSP_Execute(Serial.println(F("  Spinning")));
    counter_ = 0;
  }
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) {}

  uint8_t counter_ = 0;
  const uint8_t spinningLength_ = 40;
};

WashingmachineSm statemachine;

void setup() {
  BSP_Execute(Serial.begin(9600));
  BSP_Execute(pinMode(LED_BUILTIN, OUTPUT));
  statemachine.begin();
}

void loop() {
  statemachine.dispatch<Trigger::Timeout>();
  BSP_Execute(delay(100));
}
