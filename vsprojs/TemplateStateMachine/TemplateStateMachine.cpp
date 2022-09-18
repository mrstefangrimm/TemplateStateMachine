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
#define IAMWORKSTATION 1

#include "..\..\src\tsm.h"
#include "..\..\src\choice.h"

#include <iostream>

using namespace tsmlib;
using namespace Loki;

typedef State<VirtualGetTypeIdStateComperator, false> StateType;
typedef FactorCreator<StateType, false> StateTypeCreationPolicyType;

struct ToSimFromCalibAction {
  template<typename T>
  void perform(T*) {
  }
};

struct ToSimFromCalibGuard {
  template<typename T>
  bool eval(T* activeState) { return true; }
};

enum Triggers {
  On,
  Calibrate,
  Positionstream,
  Timeout,
  Remote,
  Manual,
  OutofSimulation,
};

struct SimulationInit : StateType, FactorCreator<SimulationInit> {
  uint8_t getTypeId() const override { return 10; }
  template<uint8_t T>
  bool _entry() { return false; }
  void _exit() { }
  template<uint8_t N>
  StateType* _doit() {
    return 0;
  }
};

struct SimulationManual : StateType, FactorCreator<SimulationManual> {
  uint8_t getTypeId() const override { return 11; }
  template<uint8_t T>
  bool _entry() { return false; }
  void _exit() { }
  template<uint8_t N>
  StateType* _doit() {
    return 0;
  }
};

struct SimulationRemote : StateType, FactorCreator<SimulationRemote> {
  uint8_t getTypeId() const override { return 12; }
  template<uint8_t T>
  bool _entry() { return false; }
  void _exit() { }
  template<uint8_t N>
  StateType* _doit() {
    return 0;
  }
};

struct Simulation;
struct Calibration;

typedef Transition<Triggers::On, Simulation, Calibration, StateTypeCreationPolicyType, ToSimFromCalibGuard, ToSimFromCalibAction> ToSimFromCalib;
typedef Transition<Triggers::Calibrate, Calibration, Simulation, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToCalibToSim;
typedef SelfTransition<Triggers::Positionstream, Simulation, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToSimFromSimPositionStream;
typedef SelfTransition<Triggers::Positionstream, Calibration, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToCalibFromCalibPositionStream;

typedef Declaration<Triggers::Timeout, Simulation, StateTypeCreationPolicyType> ToSimFromSimTimeout;
typedef Declaration<Triggers::Remote, Simulation, StateTypeCreationPolicyType> ToSimFromSimRemote;
typedef Declaration<Triggers::Manual, Simulation, StateTypeCreationPolicyType> ToSimFromSimManual;
typedef ExitDeclaration<Triggers::OutofSimulation, Calibration, Simulation, StateTypeCreationPolicyType, OkGuard> ToCalibFromSimManual;

typedef
Typelist<ToSimFromCalib,
  Typelist<ToCalibToSim,
  Typelist<ToSimFromSimPositionStream,
  Typelist<ToCalibFromCalibPositionStream,
  Typelist<ToSimFromSimTimeout,
  Typelist<ToSimFromSimRemote,
  Typelist<ToSimFromSimManual,
  Typelist<ToCalibFromSimManual,
  NullType>>>>>>>> TransitionList;

typedef
Typelist< Simulation,
  Typelist< Calibration,
  NullType>> StateList;

typedef InitialTransition<Simulation, StateTypeCreationPolicyType, EmptyAction> InitTransition;
typedef Statemachine<
  TransitionList,
  //StateList,
  InitTransition,
  NullEndTransition<StateTypeCreationPolicyType>> Sm;

struct ChoiceGuardRemoteDummy {
  template<typename T>
  bool eval(T*) {
    // true => SimulationRemote
    return true;
  }
};
struct ChoiceGuardManualDummy {
  template<typename T>
  bool eval(T*) {
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

typedef Transition<Triggers::Timeout, SimulationManual, SimulationInit, StateTypeCreationPolicyType, ChoiceGuardManualDummy, EmptyAction> ToManualFromInit;
typedef Transition<Triggers::Timeout, SimulationRemote, SimulationInit, StateTypeCreationPolicyType, ChoiceGuardRemoteDummy, EmptyAction> ToRemoteFromInit;
typedef Transition<Triggers::Remote, SimulationRemote, SimulationManual, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToRemoteFromManual;
typedef Transition<Triggers::Manual, SimulationManual, SimulationRemote, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToManualFromRemote;
typedef Transition<Triggers::OutofSimulation, Calibration, SimulationManual, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToCalibrationFromManual;

typedef
Typelist <
  ToManualFromInit,
  Typelist < ToRemoteFromInit,
  Typelist < ToRemoteFromManual,
  Typelist<ToManualFromRemote,
  Typelist<ToCalibrationFromManual,
  NullType>>>>> SimulationTransitionList;

typedef
Typelist< SimulationInit,
  Typelist< SimulationManual,
  Typelist<SimulationRemote,
  NullType>>> SimulationStateList;

typedef InitialTransition<SimulationInit, StateTypeCreationPolicyType, EmptyAction> SimulationSubstatesInitTransition;
typedef Statemachine<
  SimulationTransitionList,
  SimulationSubstatesInitTransition,
  NullEndTransition<StateTypeCreationPolicyType>> SimulationSubstatemachine;

struct Simulation : SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>, SingletonCreator<Simulation> {
  uint8_t getTypeId() const override { return 1; }

private:
  friend class SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>;

  void entry() {
  }
  void exit() {
  }
  template<uint8_t N>
  void doit() {
    if (N == Triggers::Positionstream) {
      isPositionStreamActive_ = !isPositionStreamActive_;
    }
  }

private:
  bool isPositionStreamActive_ = false;
};

struct Calibration : SimpleState<Calibration, StateType>, FactorCreator<Calibration> {
  uint8_t getTypeId() const override { return 2; }

private:
  friend class SimpleState<Calibration, StateType>;
  void entry() { }
  void exit() { }
  template<uint8_t N>
  void doit() {
  }
};

//template <class TList> struct Find;
//template <> struct Find<NullType>
//{
//  enum { value = -1 };
//};
//template <class T, class U>
//struct Find< Typelist<T, U> >
//{
//  enum { value = 1 + (is_same<bool, T>().value ? Find<NullType>::value : Find<U>::value) };
//};



int main()
{
  //typedef
  //Typelist<bool,
  //  Typelist<short,
  //  Typelist<int,
  //  Typelist<double,
  //  NullType>>>> TL;

  //const int index = Find<TL>::value;

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

  result = stateMachine.dispatch<Triggers::OutofSimulation>();

  //stateMachine.end();

  result = stateMachine.dispatch<Triggers::Calibrate>();
  // Active state: Calibration
}
