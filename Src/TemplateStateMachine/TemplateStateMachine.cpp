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
#define IAMWINDOWS 1

#include "tsmlib/tsm.h"
#include <iostream>

using namespace tsmlib;

typedef State<VirtualGetTypeIdStateComperator, false> StateType;

struct ToSonFromSoffAction {
  template<typename T>
  void perform(T*) {
  }
};

struct ToSonFromSoffGuard {
  template<typename T>
  bool check(T* activeState) { return true; }
};

enum Triggers {
  On,
  Calibrate,
  Positionstream,
  Timeout,
  Remote,
  Manual
};

struct SimulationInit : StateType, FactorCreator<SimulationInit> {
  uint8_t getTypeId() const override { return 10; }
  void entry() { }
  void exit() { }
  template<uint8_t N>
  void doit() {
  }
};

struct SimulationManual : StateType, FactorCreator<SimulationManual> {
  uint8_t getTypeId() const override { return 11; }
  void entry() { }
  void exit() { }
  template<uint8_t N>
  void doit() {
  }
};

struct SimulationRemote : StateType, FactorCreator<SimulationRemote> {
  uint8_t getTypeId() const override { return 12; }
  void entry() { }
  void exit() { }
  template<uint8_t N>
  void doit() {
  }
};

struct ChoiceGuardRemoteDummy {
  template<typename T>
  bool check(T*) {
    // true => SimulationRemote
    return true;
  }
};
struct ChoiceGuardManualDummy {
  template<typename T>
  bool check(T*) {
    // true => SimulationManual
    return true;
  }
};
typedef Choice<Triggers::Timeout, StateType, SimulationManual, SimulationRemote, SimulationInit, ChoiceGuardRemoteDummy, EmptyAction> InitChoice;

typedef Transition<Triggers::Timeout, StateType, SimulationManual, SimulationInit, ChoiceGuardManualDummy, EmptyAction> ToManualFromInit;
typedef Transition<Triggers::Timeout, StateType, SimulationRemote, SimulationInit, ChoiceGuardRemoteDummy, EmptyAction> ToRemoteFromInit;
typedef Transition<Triggers::Remote, StateType, SimulationRemote, SimulationManual, EmptyGuard, EmptyAction> ToRemoteFromManual;
typedef Transition<Triggers::Manual, StateType, SimulationManual, SimulationRemote, EmptyGuard, EmptyAction> ToManualFromRemote;

typedef
Typelist<
  ToManualFromInit,
  Typelist<ToRemoteFromInit,
  Typelist<ToRemoteFromManual,
  Typelist<ToManualFromRemote,
  NullType>>>> SimulationTransitionList;


typedef InitialTransition<StateType, SimulationInit, EmptyAction> SimulationNestedInitTransition;
typedef Statemachine<
  StateType,
  SimulationTransitionList,
  SimulationNestedInitTransition,
  NullFinalTransition<StateType>> SimulationNestedSM;

struct Simulation : StateType, SingletonCreator<Simulation> {
  uint8_t getTypeId() const override { return 1; }
  void entry() {
    _nestedSM.begin();
  }
  void exit() {
    _nestedSM.end();
  }
  template<uint8_t N>
  void doit() {

    // Check if nested consumes the trigger
    if (_nestedSM.trigger<N>().consumed) return;

    if (N == Triggers::Positionstream) {
      _isPositionStreamActive = !_isPositionStreamActive;
    }
  }

private:
  SimulationNestedSM _nestedSM;
  bool _isPositionStreamActive = false;
};

struct Calibration : StateType, FactorCreator<Calibration> {
  uint8_t getTypeId() const override { return 2; }
  void entry() { }
  void exit() { }
  template<uint8_t N>
  void doit() {
  }
};


typedef Transition<Triggers::On, StateType, Simulation, Calibration, ToSonFromSoffGuard, ToSonFromSoffAction> ToSimFromCalib;
typedef Transition<Triggers::Calibrate, StateType, Calibration, Simulation, EmptyGuard, EmptyAction> ToCalibToSim;
typedef Transition<Triggers::Positionstream, StateType, Simulation, Simulation, EmptyGuard, EmptyAction> ToSimFromSimPositionStream;
typedef Transition<Triggers::Positionstream, StateType, Calibration, Calibration, EmptyGuard, EmptyAction> ToCalibFromCalibPositionStream;
typedef Transition<Triggers::Timeout, StateType, Simulation, Simulation, EmptyGuard, EmptyAction> ToSimFromSimTimeout;
typedef Transition<Triggers::Remote, StateType, Simulation, Simulation, EmptyGuard, EmptyAction> ToSimFromSimRemote;
typedef Transition<Triggers::Manual, StateType, Simulation, Simulation, EmptyGuard, EmptyAction> ToSimFromSimManual;

typedef
Typelist<ToSimFromCalib,
  Typelist<ToCalibToSim,
  Typelist<ToSimFromSimPositionStream,
  Typelist<ToCalibFromCalibPositionStream,
  Typelist<ToSimFromSimTimeout,
  Typelist<ToSimFromSimRemote,
  Typelist<ToSimFromSimManual,
  NullType>>>>>>> TransitionList;


typedef InitialTransition<StateType, Simulation, EmptyAction> InitTransition;
typedef Statemachine<
  StateType,
  TransitionList,
  InitTransition,
  NullFinalTransition<StateType>> SM;


int main()
{

  //typedef typename TypeAt<TriggerList, 2>::Result SomeType;

  //if (SomeType::type == 1) {
  //  SomeType().trigger(0);
  //}
  //else {
  //  typedef typename TypeAt2<SomeType, 0>::Result SomeType2;
  //  SomeType2().trigger(0);
  //}

  Simulation simulation;
  Calibration calibration;

  SM stateMachine(true);
  // Active state: Simulation/Init, Position stream inactive

  auto result = stateMachine.trigger<Triggers::Positionstream>();
  // Active state: Simulation/Init, Position stream active

  result = stateMachine.trigger<Triggers::Timeout>();
  // Active state: Simulation/Remote, ChoiceGuardRemoteDummy returned true

  result = stateMachine.trigger<Triggers::Manual>();
  // Active state: Simulation/Manual

  result = stateMachine.trigger<Triggers::Calibrate>();
  // Active state: Calibration
}
