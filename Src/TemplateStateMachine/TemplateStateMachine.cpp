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
using namespace Loki;

typedef State<VirtualGetTypeIdStateComperator, false> StateType;

struct ToSimFromCalibAction {
  template<typename T>
  void perform(T*) {
  }
};

struct ToSimFromCalibGuard {
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

struct Simulation;
struct Calibration;

typedef Transition<Triggers::On, StateType, Simulation, Calibration, ToSimFromCalibGuard, ToSimFromCalibAction, false> ToSimFromCalib;
typedef Transition<Triggers::Calibrate, StateType, Calibration, Simulation, OkGuard, EmptyAction, false> ToCalibToSim;
typedef Transition<Triggers::Positionstream, StateType, Simulation, Simulation, OkGuard, EmptyAction, false> ToSimFromSimPositionStream;
typedef Transition<Triggers::Positionstream, StateType, Calibration, Calibration, OkGuard, EmptyAction, false> ToCalibFromCalibPositionStream;
typedef Transition<Triggers::Timeout, StateType, Simulation, Simulation, OkGuard, EmptyAction, false> ToSimFromSimTimeout;
typedef Transition<Triggers::Remote, StateType, Simulation, Simulation, OkGuard, EmptyAction, false> ToSimFromSimRemote;
typedef Transition<Triggers::Manual, StateType, Simulation, Simulation, OkGuard, EmptyAction, false> ToSimFromSimManual;

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
  NullFinalTransition<StateType>,
  EmptyState<StateType>> Sm;

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
typedef Choice<
  Triggers::Timeout,
  StateType,
  SimulationManual,
  SimulationRemote,
  SimulationInit,
  EmptyAction,
  ChoiceGuardRemoteDummy> InitChoice;

typedef Transition<Triggers::Timeout, StateType, SimulationManual, SimulationInit, ChoiceGuardManualDummy, EmptyAction, false> ToManualFromInit;
typedef Transition<Triggers::Timeout, StateType, SimulationRemote, SimulationInit, ChoiceGuardRemoteDummy, EmptyAction, false> ToRemoteFromInit;
typedef Transition<Triggers::Remote, StateType, SimulationRemote, SimulationManual, OkGuard, EmptyAction, false> ToRemoteFromManual;
typedef Transition<Triggers::Manual, StateType, SimulationManual, SimulationRemote, OkGuard, EmptyAction, false> ToManualFromRemote;

typedef
Typelist<
  ToManualFromInit,
  Typelist<ToRemoteFromInit,
  Typelist<ToRemoteFromManual,
  Typelist<ToManualFromRemote,
  NullType>>>> SimulationTransitionList;

typedef InitialTransition<StateType, SimulationInit, EmptyAction> SimulationSubstatesInitTransition;
typedef Statemachine<
  StateType,
  SimulationTransitionList,
  SimulationSubstatesInitTransition,
  NullFinalTransition<StateType>,
  Simulation> SimulationSubstatemachine;

struct Simulation : SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>, SingletonCreator<Simulation> {
  uint8_t getTypeId() const override { return 1; }

private:
  friend class SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>;

  void entry_() {
  }
  void exit_() {
  }
  template<uint8_t N>
  void doit_() {
    if (N == Triggers::Positionstream) {
      _isPositionStreamActive = !_isPositionStreamActive;
    }
  }

private:
  bool _isPositionStreamActive = false;
};

struct Calibration : SimpleState<Calibration, StateType>, FactorCreator<Calibration> {
  uint8_t getTypeId() const override { return 2; }

private:
  friend class SimpleState<Calibration, StateType>;
  void entry_() { }
  void exit_() { }
  template<uint8_t N>
  void doit_() {
  }
};

int main()
{
  Simulation simulation;
  Calibration calibration;

  Sm stateMachine;
  auto result = stateMachine.begin();
  // Active state: Simulation/Init, Position stream inactive

  result = stateMachine.dispatch<Triggers::Positionstream>();
  // Active state: Simulation/Init, Position stream active

  result = stateMachine.dispatch<Triggers::Timeout>();
  // Active state: Simulation/Remote, ChoiceGuardRemoteDummy returned true

  result = stateMachine.dispatch<Triggers::Manual>();
  // Active state: Simulation/Manual

  stateMachine.end();

  result = stateMachine.dispatch<Triggers::Calibrate>();
  // Active state: Calibration
}
