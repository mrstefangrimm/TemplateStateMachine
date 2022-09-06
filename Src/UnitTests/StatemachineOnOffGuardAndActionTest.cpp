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
#include "CppUnitTest.h"

#include "tsmlib/state.h"
#include "tsmlib/templatemeta.h"
#include "tsmlib/statemachine.h"
#include "tsmlib/transition.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace tsmlib;
using namespace std;

namespace UnitTests {

  typedef State<VirtualGetTypeIdStateComperator, false> StateType;

  template<typename TO, typename FROM>
  struct ActionSpy {
    static int Calls;
    template<typename T>
    void perform(T* activeState) {
      FROM* from = FROM::CreatorType::Create();
      Assert::IsTrue(activeState->equals(*from));
      FROM::CreatorType::Delete(from);
      Calls++;
    }
  };
  template<typename TO, typename FROM> int ActionSpy<TO, FROM>::Calls = 0;

  template<typename TO, typename FROM>
  struct GuardDummy {
    static int Calls;
    static bool CheckReturnValue;
    template<typename T>
    bool check(T* activeState) {
      FROM* from = FROM::CreatorType::Create();
      Assert::IsTrue(activeState->equals(*from));
      FROM::CreatorType::Delete(from);
      Calls++;
      return CheckReturnValue;
    }
  };
  template<typename TO, typename FROM> int GuardDummy<TO, FROM>::Calls = 0;
  template<typename TO, typename FROM> bool GuardDummy<TO, FROM>::CheckReturnValue = true;

  typedef ActionSpy<struct OnState, struct OffState> ToOnFromOffActionSpy;
  typedef ActionSpy<struct OffState, struct OnState> ToOffFromOnActionSpy;
  typedef ActionSpy<struct OnState, struct OnState> ToOnFromOnActionSpy;
  typedef ActionSpy<struct OffState, struct OffState> ToOffFromOffActionSpy;

  typedef GuardDummy<struct OnState, struct OffState> ToOnFromOffGuardDummy;
  typedef GuardDummy<struct OffState, struct OnState> ToOffFromOnGuardDummy;
  typedef GuardDummy<struct OnState, struct OnState> ToOnFromOnGuardDummy;
  typedef GuardDummy<struct OffState, struct OffState> ToOffFromOffGuardDummy;

  enum Triggers {
    On,
    Off,
    OnToOn,
    OffToOff
  };

  struct OnState : StateType, FactorCreator<OnState> {
    uint8_t getTypeId() const override { return 1; }
    void entry() { }
    void exit() { }
    void doit() { }
  };

  struct OffState : StateType, FactorCreator<OffState> {
    uint8_t getTypeId() const override { return 2; }
    void entry() { }
    void exit() { }
    void doit() { }
  };

  typedef Transition<Triggers::On, StateType, OnState, OffState, ToOnFromOffGuardDummy, ToOnFromOffActionSpy> ToOnFromOffTransition;
  typedef Transition<Triggers::Off, StateType, OffState, OnState, ToOffFromOnGuardDummy, ToOffFromOnActionSpy> ToOffFromOnTransition;
  typedef Transition<Triggers::OnToOn, StateType, OnState, OnState, ToOnFromOnGuardDummy, ToOnFromOnActionSpy> ToOnFromOnTransition;
  typedef Transition<Triggers::OffToOff, StateType, OffState, OffState, ToOffFromOffGuardDummy, ToOffFromOffActionSpy> ToOffFromOffTransition;

  typedef
    Typelist<ToOnFromOffTransition,
    Typelist<ToOffFromOnTransition,
    Typelist<ToOnFromOnTransition,
    Typelist<ToOffFromOffTransition,
    NullType>>>> TransitionList;

  typedef InitialTransition<StateType, OffState, EmptyAction> InitTransition;
  typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

  TEST_CLASS(StatemachineOnOffGuardAndActionTest)
  {
  public:

    TEST_METHOD(GuardsAndActions_Roundtrip)
    {
      OnState on;
      OffState off;
      Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOffActionSpy::Calls);

      SM sm;
      // Off <- Off, internal transition
      StateType* state = sm.trigger<Triggers::OffToOff>();
      Assert::AreEqual<int>(off.getTypeId(), state->getTypeId());
      Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // Off <- Off, unhandled trigger
      state = sm.trigger<Triggers::Off>();
      Assert::AreEqual<int>(off.getTypeId(), state->getTypeId());
      Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // Off <- Off, guard = false
      ToOnFromOffGuardDummy::CheckReturnValue = false;
      state = sm.trigger<Triggers::On>();
      Assert::AreEqual<int>(off.getTypeId(), state->getTypeId());
      Assert::AreEqual<int>(1, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // On <- Off
      ToOnFromOffGuardDummy::CheckReturnValue = true;
      state = sm.trigger<Triggers::On>();
      Assert::AreEqual<int>(on.getTypeId(), state->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // On <- On, internal transition
      state = sm.trigger<Triggers::OnToOn>();
      Assert::AreEqual<int>(on.getTypeId(), state->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // On <- On, unhandled trigger
      state = sm.trigger<Triggers::On>();
      Assert::AreEqual<int>(on.getTypeId(), state->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // Off <- On, unhandled trigger
      state = sm.trigger<Triggers::Off>();
      Assert::AreEqual<int>(off.getTypeId(), state->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);
    }
  };
}
