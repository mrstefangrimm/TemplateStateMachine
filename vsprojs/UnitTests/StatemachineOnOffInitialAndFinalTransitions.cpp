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

#include "CppUnitTest.h"

#include "../../src/tsm.h"
#include "TestHelpers.h"

#define CAT(A, B) A##B
#define WSTRING(A) CAT(L, #A)

namespace UT {
  namespace StatemachineOnOff {

    namespace StatemachineOnOffInitialAndFinalTransitionsImpl {

      using namespace Microsoft::VisualStudio::CppUnitTestFramework;
      using namespace tsmlib;
      using namespace UnitTests::Helpers;

      typedef State<VirtualGetTypeIdStateComparator, false> StateType;
      typedef FactoryCreator<StateType, false> StateTypeCreationPolicyType;

      namespace Trigger
      {
        struct On {};
        struct Off {};
        struct Goodbye {};
      }

      struct OnState : BasicState<OnState, StateType>, FactoryCreator<OnState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<OnState, StateType>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int OnState::entryCalls = 0;
      int OnState::exitCalls = 0;
      int OnState::doitCalls = 0;

      struct OffState : BasicState<OffState, StateType>, FactoryCreator<OffState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 2; }

      private:
        friend class BasicState<OffState, StateType>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int OffState::entryCalls = 0;
      int OffState::exitCalls = 0;
      int OffState::doitCalls = 0;

      typedef GuardDummy<StateType, EmptyState<StateType>, OffState> ToFinalFromOffGuardDummy;
      typedef GuardDummy<StateType, EmptyState<StateType>, OnState> ToFinalFromOnGuardDummy;
      typedef ActionStub<EmptyState<StateType>, struct OffState> ToFinalFromOffActionSpy;
      typedef ActionStub<EmptyState<StateType>, struct OnState> ToFinalFromOnActionSpy;
      typedef Transition<Trigger::On, OnState, OffState, StateTypeCreationPolicyType, NoGuard, NoAction> ToOnFromOffTransition;
      typedef Transition<Trigger::Off, OffState, OnState, StateTypeCreationPolicyType, NoGuard, NoAction> ToOffFromOnTransition;
      typedef FinalTransitionExplicit<Trigger::Goodbye, OffState, StateTypeCreationPolicyType, ToFinalFromOffGuardDummy, ToFinalFromOffActionSpy> ToFinalFromOffTransition;
      typedef FinalTransitionExplicit<Trigger::Goodbye, OnState, StateTypeCreationPolicyType, ToFinalFromOnGuardDummy, ToFinalFromOnActionSpy> ToFinalFromOnTransition;
      typedef FinalTransition<OffState, StateTypeCreationPolicyType> ToEndTransition;

      typedef
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToFinalFromOffTransition,
        Typelist<ToFinalFromOnTransition,
        Typelist<ToEndTransition,
        NullType>>>>> TransitionList;

      typedef ActionStub<struct OffState, struct EmptyState<StateType>> ToInitActionSpy;
      typedef InitialTransition<OffState, StateTypeCreationPolicyType, ToInitActionSpy> ToInitTransition;
      typedef Statemachine<
        TransitionList,
        ToInitTransition> Sm;
    }

    TEST_CLASS(StatemachineOnOffInitialAndFinalTransitions)
    {
    public:
      TEST_METHOD_INITIALIZE(Initialize)
      {
        reset();
      }

      TEST_METHOD(InitialTransition_ToOffState_ActionEntryDoAreCalledAndFinalizeIsIgnored)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        Sm sm;

        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOff_ExitAndActionAreCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        Sm sm;
        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        reset();
        ToFinalFromOffGuardDummy::CheckReturnValue = true;

        sm.dispatch<Trigger::Goodbye>();
        Assert::AreEqual<int>(1, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(1, ToFinalFromOffActionSpy::calls, WSTRING("1, ToFinalFromOffActionSpy::calls"));
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(1, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOffButGuardBlocks_ExitAndActionAreNotCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        Sm sm;
        sm.begin();
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        reset();
        sm.dispatch<Trigger::Goodbye>();
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(1, ToFinalFromOffActionSpy::calls, WSTRING("1, ToFinalFromOffActionSpy::calls"));
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(1, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOn_ExitAndActionAreCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        Sm sm;
        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        // On <- Off
        reset();
        sm.dispatch<Trigger::On>();
        Assert::AreEqual<int>(1, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(1, OnState::entryCalls);
        Assert::AreEqual<int>(1, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        reset();
        ToFinalFromOnGuardDummy::CheckReturnValue = true;
        sm.dispatch<Trigger::Goodbye>();
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(1, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(1, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(1, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(End_FromStateOffAndEndTransitionDefined_StateOffExitIsCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        Sm sm;
        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        reset();
        result = sm.end();
        Assert::IsTrue(result.consumed);
        Assert::IsNull(result.activeState);
        Assert::AreEqual<int>(1, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(End_FromStateOnAndEndTransitionNotDefined_StateOnExitIsNotCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        Sm sm;
        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        // On <- Off
        reset();
        sm.dispatch<Trigger::On>();
        Assert::AreEqual<int>(1, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(1, OnState::entryCalls);
        Assert::AreEqual<int>(1, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        reset();
        result = sm.end();
        Assert::IsFalse(result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(End_FromStateOffAndEndTransitionDefinedAndGuardBlocks_ActiveStateRemainsActive)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        Sm sm;
        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);

        sm.dispatch<Trigger::Off>();
        reset();

        result = sm.end();
        Assert::IsTrue(result.consumed);
        Assert::IsNull(result.activeState);
        Assert::AreEqual<int>(1, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, ToInitActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::calls);
      }

    private:
      void reset() {
        using namespace StatemachineOnOffInitialAndFinalTransitionsImpl;
        OffState::exitCalls = 0;
        OffState::entryCalls = 0;
        OffState::doitCalls = 0;
        OnState::exitCalls = 0;
        OnState::entryCalls = 0;
        OnState::doitCalls = 0;
        ToInitActionSpy::calls = 0;
        ToFinalFromOffActionSpy::calls = 0;
        ToFinalFromOnActionSpy::calls = 0;
        ToFinalFromOffGuardDummy::calls = 0;
        ToFinalFromOnGuardDummy::calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
        ToFinalFromOnGuardDummy::CheckReturnValue = false;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
      }
    };
  }
}
