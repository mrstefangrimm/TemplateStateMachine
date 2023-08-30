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
#include "../../src/tsm.h"
#include "TestHelpers.h"

namespace UT {
  namespace StatemachineOnOff {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace StatemachineOnOffEntryCallsExitTestImpl {

      using StatePolicy = State<VirtualTypeIdComparator, false>;

      namespace Trigger
      {
        struct On {};
        struct Off {};
        struct OnToOn {};
        struct OffToOff {};
      }

      struct OnState : BasicState<OnState, StatePolicy, true, true, true>, FactoryCreatorFake<OnState> {
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

      struct OffState : BasicState<OffState, StatePolicy, true, true, true>, FactoryCreatorFake<OffState> {
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

      using ToOnFromOffTransition = Transition<Trigger::On, OnState, OffState, NoGuard, NoAction>;
      using ToOffFromOnTransition = Transition<Trigger::Off, OffState, OnState, NoGuard, NoAction>;
      using ToOnFromOnTransition = SelfTransition<Trigger::OnToOn, OnState, NoGuard, NoAction, false>;
      using ToOffFromOffTransition = SelfTransition<Trigger::OffToOff, OffState, NoGuard, NoAction, false>;

      using TransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToOnFromOnTransition,
        Typelist<ToOffFromOffTransition,
        NullType>>>>;

      using InitTransition = InitialTransition<OffState, NoAction>;
      using Sm = Statemachine<TransitionList, InitTransition>;
    }

    TEST_CLASS(StatemachineOnOffEntryDoExitTest)
    {
      TEST_METHOD_INITIALIZE(Initialize)
      {
        reset();
      }

      TEST_METHOD(EntriesDoesExits_Roundtrip)
      {
        using namespace StatemachineOnOffEntryCallsExitTestImpl;
        OnState on;
        OffState off;

        Sm sm;
        sm.begin();
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);

        // Off <- Off, self transition
        reset();
        auto result = sm.dispatch<Trigger::OffToOff>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);

        // Off <- Off, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::Off>();
        Assert::IsTrue(result.activeState->typeOf<OffState>());
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);

        // On <- Off
        reset();
        result = sm.dispatch<Trigger::On>();
        Assert::IsTrue(result.activeState->typeOf<OnState>());
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(1, OnState::entryCalls);
        Assert::AreEqual<int>(1, OnState::doitCalls);
        Assert::AreEqual<int>(1, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);

        // On <- On, self transition
        reset();
        result = sm.dispatch<Trigger::OnToOn>();
        Assert::IsTrue(result.activeState->typeOf<OnState>());
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(1, OnState::doitCalls);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);

        // On <- On, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::On>();
        Assert::IsTrue(result.activeState->typeOf<OnState>());
        Assert::AreEqual<int>(0, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(0, OffState::entryCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);

        // Off <- On, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::Off>();
        Assert::IsTrue(result.activeState->typeOf<OffState>());
        Assert::AreEqual<int>(1, OnState::exitCalls);
        Assert::AreEqual<int>(0, OnState::entryCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
        Assert::AreEqual<int>(0, OffState::exitCalls);
        Assert::AreEqual<int>(1, OffState::entryCalls);
        Assert::AreEqual<int>(1, OffState::doitCalls);

        // Active state is Off
        Assert::AreEqual<int>(FactoryCreatorFake<OffState>::createCalls, FactoryCreatorFake<OffState>::deleteCalls + 1);
        Assert::AreEqual<int>(FactoryCreatorFake<OnState>::createCalls, FactoryCreatorFake<OnState>::deleteCalls);
      }

    private:
      void reset() const {
        using namespace StatemachineOnOffEntryCallsExitTestImpl;
        OnState::exitCalls = 0;
        OnState::entryCalls = 0;
        OnState::doitCalls = 0;
        OffState::exitCalls = 0;
        OffState::entryCalls = 0;
        OffState::doitCalls = 0;
      }
    };
  }
}
