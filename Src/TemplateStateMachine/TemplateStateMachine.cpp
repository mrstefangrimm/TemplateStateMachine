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
  bool check(T* activeState) {  return true; }
};

enum Triggers {
  On,
  Off,
  OnToOn
};


struct Son : StateType, SingletonCreator<Son> {
  uint8_t getTypeId() const override { return 1; }
  void entry() { }
  void exit() { }
  void doit() { }
  // TODO: cleanup Do(Int2Type<TRIGGER>())
  //void Do(Int2Type<Triggers::On>) { }
  //void Do(Int2Type<Triggers::OnToOn>) { }
};

struct Soff : StateType, SingletonCreator<Soff> {

  uint8_t getTypeId() const override { return 2; }
  void entry() { }
  void exit() { }
  void doit() { }
  // TODO: cleanup Do(Int2Type<TRIGGER>())
  //void Do(Int2Type<Triggers::Off>) { }
};


typedef Transition<Triggers::On, StateType, Son, Soff, ToSonFromSoffGuard, ToSonFromSoffAction> ToSonFromSoff_t;
typedef Transition<Triggers::Off, StateType, Soff, Son, EmptyGuard, EmptyAction> ToSoffFromSon_t;
typedef Transition<Triggers::OnToOn, StateType, Son, Son, EmptyGuard, EmptyAction> ToSonFromSon_t;

typedef
Typelist<ToSonFromSoff_t,
  Typelist<ToSoffFromSon_t,
  Typelist<ToSonFromSon_t,
  NullType>>> TransitionList;

typedef
Typelist<Soff,
  Typelist<Son,
  NullType>> StateList;

struct GoFinalGuard {
 template<typename T>
  bool check(T* activeState) { return true; }
};

typedef InitialTransition<StateType, Soff, EmptyAction> InitTransition;
typedef FinalTransition<StateType, GoFinalGuard, EmptyAction> TerminateTransition;
typedef Statemachine<
  StateType,
  TransitionList,
  NullStatemachine<StateType>,
  InitTransition,
  TerminateTransition /*NullFinalTransition<StateType>*/> SM;

int main()
{
  void* ptr = Son::Create();

  Son son;
  Soff soff;
  SM stateMachine(true);
  StateType* st = stateMachine.trigger<Triggers::On>();
  st = stateMachine.trigger<Triggers::OnToOn>();
  st = stateMachine.trigger<Triggers::Off>();
  st = stateMachine.trigger<Triggers::Off>();

  //
  //st = stateMachine.Trigge<Triggers::OnToOn>();

  //Transition<Triggers::Off, Son, SonFac_t, Soff, SoffFac_t, ToSonFromSoffGuard, ToSonFromSoffAction> toSonFromSoff;
  //toSonFromSoff.transition();

  //Transition<Triggers::On, Son, SonFac_t, Son, SonFac_t, EmptyGuard, EmptyAction> toSonFromSon;
  //toSonFromSon.transition();

  //Transition<Triggers::On, Soff, SoffFac_t, Son, SonFac_t, EmptyGuard, EmptyAction> toSoffFromSon;
  //toSoffFromSon.transition();

  //SOn son;
  //son.action();
  
  std::cout << "Hello World!\n";
}
