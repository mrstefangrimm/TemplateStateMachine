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

#include <iostream>

using namespace tsmlib;

typedef State<VirtualGetTypeIdStateComparator, false> StateType;
typedef FactoryCreator<StateType, false> StateTypeCreationPolicyType;

struct ToSimFromCalibAction {
  template<class T>
  void perform(T*) {
  }
};

struct ToSimFromCalibGuard {
  template<class T>
  bool eval(T* activeState) { return true; }
};

namespace Trigger {
  struct On;
  struct Calibrate;
  struct Positionstream;
  struct Timeout;
  struct Remote;
  struct Manual;
  struct OutofSimulation;
}

struct SimulationInit : BasicState<SimulationInit, StateType>, FactoryCreator<SimulationInit> {
  uint8_t getTypeId() const override { return 10; }
  void entry() { }
  void exit() { }
  template<class Event>
  void doit() { }
};

struct SimulationManual : BasicState<SimulationManual, StateType>, FactoryCreator<SimulationManual> {
  uint8_t getTypeId() const override { return 11; }
  void entry() { }
  void exit() { }
  template<class Event>
  void doit() { }
};

struct SimulationRemote : BasicState<SimulationRemote, StateType>, FactoryCreator<SimulationRemote> {
  uint8_t getTypeId() const override { return 12; }
  void entry() { }
  void exit() { }
  template<class Event>
  void doit() { }
};

struct Simulation;
struct Calibration;

typedef Transition<Trigger::On, Simulation, Calibration, StateTypeCreationPolicyType, ToSimFromCalibGuard, ToSimFromCalibAction> ToSimFromCalib;
typedef Transition<Trigger::Calibrate, Calibration, Simulation, StateTypeCreationPolicyType, NoGuard, NoAction> ToCalibToSim;
typedef SelfTransition<Trigger::Positionstream, Simulation, StateTypeCreationPolicyType, NoGuard, NoAction, false> ToSimFromSimPositionStream;
typedef SelfTransition<Trigger::Positionstream, Calibration, StateTypeCreationPolicyType, NoGuard, NoAction, false> ToCalibFromCalibPositionStream;

typedef Declaration<Trigger::Timeout, Simulation, StateTypeCreationPolicyType> ToSimFromSimTimeout;
typedef Declaration<Trigger::Remote, Simulation, StateTypeCreationPolicyType> ToSimFromSimRemote;
typedef Declaration<Trigger::Manual, Simulation, StateTypeCreationPolicyType> ToSimFromSimManual;
typedef ExitDeclaration<Trigger::OutofSimulation, Calibration, Simulation, StateTypeCreationPolicyType> ToCalibFromSimManual;

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

typedef InitialTransition<Simulation, StateTypeCreationPolicyType, NoAction> InitTransition;
typedef Statemachine<
  TransitionList,
  InitTransition> Sm;

struct ChoiceGuardRemoteDummy {
  template<class T>
  bool eval(T*) {
    // true => SimulationRemote
    return true;
  }
};
struct ChoiceGuardManualDummy {
  template<class T>
  bool eval(T*) {
    // true => SimulationManual
    return true;
  }
};

typedef Transition<Trigger::Timeout, SimulationManual, SimulationInit, StateTypeCreationPolicyType, ChoiceGuardManualDummy, NoAction> ToManualFromInit;
typedef Transition<Trigger::Timeout, SimulationRemote, SimulationInit, StateTypeCreationPolicyType, ChoiceGuardRemoteDummy, NoAction> ToRemoteFromInit;
typedef Transition<Trigger::Remote, SimulationRemote, SimulationManual, StateTypeCreationPolicyType, NoGuard, NoAction> ToRemoteFromManual;
typedef Transition<Trigger::Manual, SimulationManual, SimulationRemote, StateTypeCreationPolicyType, NoGuard, NoAction> ToManualFromRemote;
typedef Transition<Trigger::OutofSimulation, Calibration, SimulationManual, StateTypeCreationPolicyType, NoGuard, NoAction> ToCalibrationFromManual;

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

typedef InitialTransition<SimulationInit, StateTypeCreationPolicyType, NoAction> SimulationSubstatesInitTransition;
typedef Statemachine<
  SimulationTransitionList,
  SimulationSubstatesInitTransition> SimulationSubstatemachine;

struct Simulation : SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>, SingletonCreator<Simulation> {
  uint8_t getTypeId() const override { return 1; }

private:
  friend class SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>;

  void entry() {
  }
  void exit() {
  }
  template<class Event>
  void doit() {
    if (is_same<Event, Trigger::Positionstream>().value) {
      isPositionStreamActive_ = !isPositionStreamActive_;
    }
  }

private:
  bool isPositionStreamActive_ = false;
};

struct Calibration : BasicState<Calibration, StateType>, FactoryCreator<Calibration> {
  uint8_t getTypeId() const override { return 2; }

private:
  friend class BasicState<Calibration, StateType>;
  void entry() { }
  void exit() { }
  template<class Event>
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

  result = stateMachine.dispatch<Trigger::Positionstream>();
  // Active state: Simulation/Init, Position stream active

  result = stateMachine.dispatch<Trigger::Timeout>();
  // Active state: Simulation/Remote, ChoiceGuardRemoteDummy returned true

  result = stateMachine.dispatch<Trigger::Manual>();
  // Active state: Simulation/Manual

  result = stateMachine.dispatch<Trigger::OutofSimulation>();

  //stateMachine.end();

  result = stateMachine.dispatch<Trigger::Calibrate>();
  // Active state: Calibration
}
