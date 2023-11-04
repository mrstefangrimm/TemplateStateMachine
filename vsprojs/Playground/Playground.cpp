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

#include "../../src/tsm.h"

#include <iostream>

using namespace tsmlib;

struct ToSimFromCalibAction {
  template<class T, class V>
  void perform(T&, const V&) {
  }
};

struct ToSimFromCalibGuard {
  template<class T, class V>
  bool eval(const T&, const V&) { return true; }
};

namespace Trigger {
  struct On {};
  struct Calibrate {};
  struct Positionstream {};
  struct Timeout {};
  struct Remote {};
  struct Manual {};
  struct OutofSimulation {};
}

typedef State<VirtualTypeIdComparator, false> StateType;

struct SimulationInit : BasicState<SimulationInit, StateType>, FactoryCreator<SimulationInit> {
  uint8_t getTypeId() const override { return 10; }
  template<class Event> void entry(const Event&) {}
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) {}
};

struct SimulationManual : BasicState<SimulationManual, StateType>, FactoryCreator<SimulationManual> {
  uint8_t getTypeId() const override { return 11; }
  template<class Event> void entry(const Event&) {}
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) { }
};

struct SimulationRemote : BasicState<SimulationRemote, StateType>, FactoryCreator<SimulationRemote> {
  uint8_t getTypeId() const override { return 12; }
  template<class Event> void entry(const Event&) {}
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) { }
};

struct Simulation;
struct Calibration;

typedef Transition<Trigger::On, Simulation, Calibration, ToSimFromCalibGuard, ToSimFromCalibAction> ToSimFromCalib;
typedef Transition<Trigger::Calibrate, Calibration, Simulation, NoGuard, NoAction> ToCalibToSim;
typedef SelfTransition<Trigger::Positionstream, Simulation, NoGuard, NoAction, false> ToSimFromSimPositionStream;
typedef SelfTransition<Trigger::Positionstream, Calibration, NoGuard, NoAction, false> ToCalibFromCalibPositionStream;

typedef Declaration<Trigger::Timeout, Simulation> ToSimFromSimTimeout;
typedef Declaration<Trigger::Remote, Simulation> ToSimFromSimRemote;
typedef Declaration<Trigger::Manual, Simulation> ToSimFromSimManual;
typedef ExitDeclaration<Trigger::OutofSimulation, Calibration, Simulation> ToCalibFromSimManual;

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

typedef InitialTransition<Simulation, NoAction> InitTransition;
typedef Statemachine<
  TransitionList,
  InitTransition> Sm;

struct ChoiceGuardRemoteDummy {
  template<class StateType, class EventType>
  bool eval(const StateType&, const EventType&) {
    // true => SimulationRemote
    return true;
  }
};

struct ChoiceGuardManualDummy {
  template<class StateType, class EventType>
  bool eval(const StateType&, const EventType&) {
    // true => SimulationManual
    return true;
  }
};

typedef Transition<Trigger::Timeout, SimulationManual, SimulationInit, ChoiceGuardManualDummy, NoAction> ToManualFromInit;
typedef Transition<Trigger::Timeout, SimulationRemote, SimulationInit, ChoiceGuardRemoteDummy, NoAction> ToRemoteFromInit;
typedef Transition<Trigger::Remote, SimulationRemote, SimulationManual, NoGuard, NoAction> ToRemoteFromManual;
typedef Transition<Trigger::Manual, SimulationManual, SimulationRemote, NoGuard, NoAction> ToManualFromRemote;
typedef Transition<Trigger::OutofSimulation, Calibration, SimulationManual, NoGuard, NoAction> ToCalibrationFromManual;

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

typedef InitialTransition<SimulationInit, NoAction> SimulationSubstatesInitTransition;
typedef Statemachine<
  SimulationTransitionList,
  SimulationSubstatesInitTransition> SimulationSubstatemachine;

struct Simulation : SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>, FactoryCreator<Simulation> {
  uint8_t getTypeId() const override { return 1; }

private:
  friend class SubstatesHolderState<Simulation, StateType, SimulationSubstatemachine>;

  template<class Event> void entry(const Event&) {}
  template<class Event> void exit(const Event&) {}
  template<class Event> void doit(const Event&) {
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
  template<class Event> void entry(const Event&) { }
  template<class Event> void exit(const Event&) { }
  template<class Event> void doit(const Event&) {
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
