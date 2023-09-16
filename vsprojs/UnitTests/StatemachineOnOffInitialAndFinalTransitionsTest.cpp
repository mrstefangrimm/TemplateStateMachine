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
#include "CppUnitTest.h"
#include "NotquiteBDD.h"
#include "../../src/tsm.h"
#include "TestHelpers.h"

namespace UT {
  namespace StatemachineOnOff {

    namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl {

      using namespace tsmlib;
      using namespace UnitTests::Helpers;

      using StatePolicy = State<VirtualTypeIdComparator, false>;

      namespace Trigger
      {
        struct On {};
        struct Off {};
        struct Goodbye {};
      }

      struct OnState : BasicState<OnState, StatePolicy, true, true, true>, FactoryCreator<OnState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<OnState, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int OnState::entryCalls = 0;
      int OnState::exitCalls = 0;
      int OnState::doitCalls = 0;

      struct OffState : BasicState<OffState, StatePolicy, true, true, true>, FactoryCreator<OffState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 2; }

      private:
        friend class BasicState<OffState, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int OffState::entryCalls = 0;
      int OffState::exitCalls = 0;
      int OffState::doitCalls = 0;

      using ToFinalFromOffGuardDummy = GuardDummy<StatePolicy, EmptyState<StatePolicy>, OffState>;
      using ToFinalFromOnGuardDummy = GuardDummy<StatePolicy, EmptyState<StatePolicy>, OnState>;
      using ToFinalFromOffActionSpy = ActionStub<EmptyState<StatePolicy>, struct OffState>;
      using ToFinalFromOnActionSpy = ActionStub<EmptyState<StatePolicy>, struct OnState>;
      using ToOnFromOffTransition = Transition<Trigger::On, OnState, OffState, NoGuard, NoAction>;
      using ToOffFromOnTransition = Transition<Trigger::Off, OffState, OnState, NoGuard, NoAction>;
      using ToFinalFromOffTransition = FinalTransitionExplicit<Trigger::Goodbye, OffState, ToFinalFromOffGuardDummy, ToFinalFromOffActionSpy>;
      using ToFinalFromOnTransition = FinalTransitionExplicit<Trigger::Goodbye, OnState, ToFinalFromOnGuardDummy, ToFinalFromOnActionSpy>;
      using ToEndTransition = FinalTransition<OffState>;

      using TransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToFinalFromOffTransition,
        Typelist<ToFinalFromOnTransition,
        Typelist<ToEndTransition,
        NullType>>>>>;

      using ToInitActionSpy = ActionStub<struct OffState, struct EmptyState<StatePolicy>>;
      using ToInitTransition = InitialTransition<OffState, ToInitActionSpy>;
      using Sm = Statemachine<TransitionList, ToInitTransition>;
    }

    BEGIN(StatemachineOnOffInitialAndFinalTransitionsTest)

      INIT(
        Initialize,
        {
          reset();
        })

      TEST_METHOD(InitialTransition_ToOffState_ActionEntryDoAreCalledAndFinalizeIsIgnored)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        Sm sm;

        auto result = sm.begin();
        EQ(true, result.consumed);
        NN(result.activeState);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToInitActionSpy::callsWithNullType);
        EQ(1, ToInitActionSpy::callsWithoutEvent);
        EQ(0, ToFinalFromOffActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOnActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOffActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOnActionSpy::callsWithEvent);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOff_ExitAndActionAreCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        Sm sm;
        auto result = sm.begin();
        EQ(true, result.consumed);
        NN(result.activeState);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToInitActionSpy::callsWithNullType);
        EQ(1, ToInitActionSpy::callsWithoutEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        reset();
        ToFinalFromOffGuardDummy::CheckReturnValue = true;

        sm.dispatch<Trigger::Goodbye>();
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(1, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(1, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOffButGuardBlocks_ExitAndActionAreNotCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        Sm sm;
        sm.begin();
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToInitActionSpy::callsWithNullType);
        EQ(1, ToInitActionSpy::callsWithoutEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        reset();
        auto result = sm.dispatch<Trigger::Goodbye>();
        TRUE(result.activeState->typeOf<OffState>());
        EQ(0, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(1, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(1, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOn_ExitAndActionAreCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        Sm sm;
        auto result = sm.begin();
        EQ(true, result.consumed);
        NN(result.activeState);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToInitActionSpy::callsWithNullType);
        EQ(1, ToInitActionSpy::callsWithoutEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        // On <- Off
        reset();
        sm.dispatch<Trigger::On>();
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(1, OnState::entryCalls);
        EQ(1, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        reset();
        ToFinalFromOnGuardDummy::CheckReturnValue = true;
        sm.dispatch<Trigger::Goodbye>();
        EQ(0, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(1, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(1, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(1, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(End_FromStateOffAndEndTransitionDefined_StateOffExitIsCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        Sm sm;
        auto result = sm.begin();
        EQ(true, result.consumed);
        NN(result.activeState);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToInitActionSpy::callsWithNullType);
        EQ(1, ToInitActionSpy::callsWithoutEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        reset();
        result = sm.end();
        TRUE(result.consumed);
        N(result.activeState);
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(End_FromStateOnAndEndTransitionNotDefined_StateOnExitIsNotCalled)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        Sm sm;
        auto result = sm.begin();
        EQ(true, result.consumed);
        NN(result.activeState);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToInitActionSpy::callsWithNullType);
        EQ(1, ToInitActionSpy::callsWithoutEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        // On <- Off
        reset();
        sm.dispatch<Trigger::On>();
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(1, OnState::entryCalls);
        EQ(1, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        reset();
        result = sm.end();
        FALSE(result.consumed);
        NN(result.activeState);
        EQ(0, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);
      }

      TEST_METHOD(End_FromStateOffAndEndTransitionDefinedAndGuardBlocks_ActiveStateRemainsActive)
      {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        Sm sm;
        auto result = sm.begin();
        EQ(true, result.consumed);
        NN(result.activeState);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToInitActionSpy::callsWithNullType);
        EQ(1, ToInitActionSpy::callsWithoutEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);

        result = sm.dispatch<Trigger::Off>();
        TRUE(result.activeState->typeOf<OffState>());

        reset();

        result = sm.end();
        TRUE(result.consumed);
        N(result.activeState);
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, ToInitActionSpy::callsWithEvent);
        EQ(0, ToFinalFromOffActionSpy::calls());
        EQ(0, ToFinalFromOnActionSpy::calls());
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOnGuardDummy::calls);
      }

      void reset() const {
        using namespace StatemachineOnOffInitialAndFinalTransitionsTestImpl;
        OffState::exitCalls = 0;
        OffState::entryCalls = 0;
        OffState::doitCalls = 0;
        OnState::exitCalls = 0;
        OnState::entryCalls = 0;
        OnState::doitCalls = 0;
        ToInitActionSpy::reset();
        ToFinalFromOffActionSpy::reset();
        ToFinalFromOnActionSpy::reset();
        ToFinalFromOffGuardDummy::calls = 0;
        ToFinalFromOnGuardDummy::calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
        ToFinalFromOnGuardDummy::CheckReturnValue = false;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
      }

    END

  }
}
