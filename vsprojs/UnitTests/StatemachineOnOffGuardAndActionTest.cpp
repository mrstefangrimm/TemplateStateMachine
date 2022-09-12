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

#include "CppUnitTest.h"

#include "..\..\src\state.h"
#include "..\..\src\templatemeta.h"
#include "..\..\src\statemachine.h"
#include "..\..\src\transition.h"
#include "TestHelpers.h"

namespace UnitTests {

  using namespace Microsoft::VisualStudio::CppUnitTestFramework;
  using namespace tsmlib;
  using namespace std;
  using namespace Helpers;

  typedef State<VirtualGetTypeIdStateComperator, false> StateType;
  typedef FactorCreator<StateType, false> StateTypeCreationPolicyType;

  typedef ActionSpy<struct OnState, struct OffState> ToOnFromOffActionSpy;
  typedef ActionSpy<struct OffState, struct OnState> ToOffFromOnActionSpy;
  typedef ActionSpy<struct OnState, struct OnState> ToOnFromOnActionSpy;
  typedef ActionSpy<struct OffState, struct OffState> ToOffFromOffActionSpy;

  typedef GuardDummy<StateType, struct OnState, struct OffState> ToOnFromOffGuardDummy;
  typedef GuardDummy<StateType, struct OffState, struct OnState> ToOffFromOnGuardDummy;
  typedef GuardDummy<StateType, struct OnState, struct OnState> ToOnFromOnGuardDummy;
  typedef GuardDummy<StateType, struct OffState, struct OffState> ToOffFromOffGuardDummy;

  enum Triggers {
    On,
    Off,
    OnToOn,
    OffToOff
  };

  struct OnState : SimpleState<OnState, StateType>, FactorCreator<OnState> {
    uint8_t getTypeId() const override { return 1; }

  private:
    friend class SimpleState<OnState, StateType>;
    void entry() { }
    void exit() { }
    template<uint8_t N>
    void doit() { }
  };

  struct OffState : SimpleState<OffState, StateType>, FactorCreator<OffState> {
    uint8_t getTypeId() const override { return 2; }

  private:
    friend class SimpleState<OffState, StateType>;
    void entry() { }
    void exit() { }
    template<uint8_t N>
    void doit() { }
  };

  typedef Transition<Triggers::On, OnState, OffState, StateTypeCreationPolicyType, ToOnFromOffGuardDummy, ToOnFromOffActionSpy> ToOnFromOffTransition;
  typedef Transition<Triggers::Off, OffState, OnState, StateTypeCreationPolicyType, ToOffFromOnGuardDummy, ToOffFromOnActionSpy> ToOffFromOnTransition;
  typedef SelfTransition<Triggers::OnToOn, OnState, StateTypeCreationPolicyType, ToOnFromOnGuardDummy, ToOnFromOnActionSpy> ToOnFromOnTransition;
  typedef SelfTransition<Triggers::OffToOff, OffState, StateTypeCreationPolicyType, ToOffFromOffGuardDummy, ToOffFromOffActionSpy> ToOffFromOffTransition;

  typedef
    Typelist<ToOnFromOffTransition,
    Typelist<ToOffFromOnTransition,
    Typelist<ToOnFromOnTransition,
    Typelist<ToOffFromOffTransition,
    NullType>>>> TransitionList;

  typedef InitialTransition<OffState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
  typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

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

      Sm sm;
      sm.begin();
      // Off <- Off, self transition
      auto result = sm.dispatch<Triggers::OffToOff>();
      Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
      Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // Off <- Off, unhandled trigger
      result = sm.dispatch<Triggers::Off>();
      Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
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
      result = sm.dispatch<Triggers::On>();
      Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
      Assert::AreEqual<int>(1, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // On <- Off
      ToOnFromOffGuardDummy::CheckReturnValue = true;
      result = sm.dispatch<Triggers::On>();
      Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(2, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // On <- On, self transition
      result = sm.dispatch<Triggers::OnToOn>();
      Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(2, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // On <- On, unhandled trigger
      result = sm.dispatch<Triggers::On>();
      Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(2, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(0, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);

      // Off <- On, unhandled trigger
      result = sm.dispatch<Triggers::Off>();
      Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
      Assert::AreEqual<int>(2, ToOnFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(2, ToOnFromOffActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOnFromOnActionSpy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::Calls);
      Assert::AreEqual<int>(1, ToOffFromOffActionSpy::Calls);
    }
  };
}
