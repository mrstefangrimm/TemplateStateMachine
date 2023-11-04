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

    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace StatemachineOnOffGuardAndActionTestImpl {

      using StatePolicy = State<VirtualTypeIdComparator, false>;
      using FinalState = EmptyState<StatePolicy>;

      struct OnState;
      struct OffState;
      using ToOnFromOffActionSpy = ActionStub<OnState, OffState>;
      using ToOffFromOnActionSpy = ActionStub<OffState, OnState>;
      using ToOnFromOnActionSpy = ActionStub<OnState, OnState>;
      using ToOffFromOffActionSpy = ActionStub<OffState, OffState>;
      using ToOffFromInitActionSpy = ActionStub<OffState, EmptyState<StatePolicy>>;
      using ToFinalFromOffActionSpy = ActionStub<FinalState, OffState>;

      using ToOnFromOffGuardDummy = GuardDummy<StatePolicy, OnState, OffState>;
      using ToOffFromOnGuardDummy = GuardDummy<StatePolicy, OffState, OnState>;
      using ToOnFromOnGuardDummy = GuardDummy<StatePolicy, OnState, OnState>;
      using ToOffFromOffGuardDummy = GuardDummy<StatePolicy, OffState, OffState>;
      using ToFinalFromOffGuardDummy = GuardDummy<StatePolicy, FinalState, OffState>;

      namespace Trigger
      {
        struct On {};
        struct Off {};
        struct OnToOn {};
        struct OffToOff {};
        struct Stop {};
      }

      struct OnState : BasicState<OnState, StatePolicy>, FactoryCreator<OnState> {
        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<OnState, StatePolicy>;
        template<class Event> void entry(const Event&) { }
        template<class Event> void exit(const Event&) { }
        template<class Event> void doit(const Event&) { }
      };

      struct OffState : BasicState<OffState, StatePolicy>, FactoryCreator<OffState> {
        uint8_t getTypeId() const override { return 2; }

      private:
        friend class BasicState<OffState, StatePolicy>;
        template<class Event> void entry(const Event&) { }
        template<class Event> void exit(const Event&) { }
        template<class Event> void doit(const Event&) { }
      };

      using ToOnFromOffTransition = Transition<Trigger::On, OnState, OffState, ToOnFromOffGuardDummy, ToOnFromOffActionSpy>;
      using ToOffFromOnTransition = Transition<Trigger::Off, OffState, OnState, ToOffFromOnGuardDummy, ToOffFromOnActionSpy>;
      using ToOnFromOnTransition = SelfTransition<Trigger::OnToOn, OnState, ToOnFromOnGuardDummy, ToOnFromOnActionSpy, false>;
      using ToOffFromOffTransition = SelfTransition<Trigger::OffToOff, OffState, ToOffFromOffGuardDummy, ToOffFromOffActionSpy, false>;
      using ToFinalFromOn = FinalTransition<OnState>;
      using ToFinalFromOff = FinalTransition<OffState>;
      using ToFinalFromOffTransition = FinalTransitionExplicit<Trigger::Stop, OffState, ToFinalFromOffGuardDummy, ToFinalFromOffActionSpy>;

      using TransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToOnFromOnTransition,
        Typelist<ToOffFromOffTransition,
        Typelist<ToFinalFromOn,
        Typelist<ToFinalFromOff,
        Typelist<ToFinalFromOffTransition,
        NullType>>>>>>>;

      using InitTransition = InitialTransition<OffState, ToOffFromInitActionSpy>;
      using Sm = Statemachine<TransitionList, InitTransition>;
    }

    BEGIN(StatemachineOnOffGuardAndActionTest)

      INIT(
        Initialize,
        {
          reset();
        })

      TEST(
        Roundtrip,
        GuardsAndActions,
        Successful)
      {
        using namespace StatemachineOnOffGuardAndActionTestImpl;
        OnState on;
        OffState off;
 
        Sm sm;
        sm.begin();
        // Off <- Off, self transition
        auto result = sm.dispatch<Trigger::OffToOff>();
        TRUE(result.activeState->typeOf<OffState>());
        EQ(0, ToOnFromOffGuardDummy::calls);
        EQ(0, ToOnFromOffActionSpy::calls());
        EQ(0, ToOffFromOnGuardDummy::calls);
        EQ(0, ToOffFromOnActionSpy::calls());
        EQ(0, ToOnFromOnGuardDummy::calls);
        EQ(0, ToOnFromOnActionSpy::calls());
        EQ(1, ToOffFromOffGuardDummy::calls);
        EQ(1, ToOffFromOffActionSpy::callsWithEvent);

        // Off <- Off, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::Off>();
        TRUE(result.activeState->typeOf<OffState>());
        EQ(0, ToOnFromOffGuardDummy::calls);
        EQ(0, ToOnFromOffActionSpy::calls());
        EQ(0, ToOffFromOnGuardDummy::calls);
        EQ(0, ToOffFromOnActionSpy::calls());
        EQ(0, ToOnFromOnGuardDummy::calls);
        EQ(0, ToOnFromOnActionSpy::calls());
        EQ(0, ToOffFromOffGuardDummy::calls);
        EQ(0, ToOffFromOffActionSpy::calls());

        // Off <- Off, guard = false
        reset();
        ToOnFromOffGuardDummy::CheckReturnValue = false;
        result = sm.dispatch<Trigger::On>();
        TRUE(result.activeState->typeOf<OffState>());
        EQ(1, ToOnFromOffGuardDummy::calls);
        EQ(1, ToOnFromOffActionSpy::callsWithEvent);
        EQ(0, ToOffFromOnGuardDummy::calls);
        EQ(0, ToOffFromOnActionSpy::calls());
        EQ(0, ToOnFromOnGuardDummy::calls);
        EQ(0, ToOnFromOnActionSpy::calls());
        EQ(0, ToOffFromOffGuardDummy::calls);
        EQ(0, ToOffFromOffActionSpy::calls());

        // On <- Off
        reset();
        ToOnFromOffGuardDummy::CheckReturnValue = true;
        result = sm.dispatch<Trigger::On>();
        TRUE(result.activeState->typeOf<OnState>());
        EQ(1, ToOnFromOffGuardDummy::calls);
        EQ(1, ToOnFromOffActionSpy::callsWithEvent);
        EQ(0, ToOffFromOnGuardDummy::calls);
        EQ(0, ToOffFromOnActionSpy::calls());
        EQ(0, ToOnFromOnGuardDummy::calls);
        EQ(0, ToOnFromOnActionSpy::calls());
        EQ(0, ToOffFromOffGuardDummy::calls);
        EQ(0, ToOffFromOffActionSpy::calls());

        // On <- On, self transition
        reset();
        result = sm.dispatch<Trigger::OnToOn>();
        TRUE(result.activeState->typeOf<OnState>());
        EQ(0, ToOnFromOffGuardDummy::calls);
        EQ(0, ToOnFromOffActionSpy::calls());
        EQ(0, ToOffFromOnGuardDummy::calls);
        EQ(0, ToOffFromOnActionSpy::calls());
        EQ(1, ToOnFromOnGuardDummy::calls);
        EQ(1, ToOnFromOnActionSpy::callsWithEvent);
        EQ(0, ToOffFromOffGuardDummy::calls);
        EQ(0, ToOffFromOffActionSpy::calls());

        // On <- On, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::On>();
        TRUE(result.activeState->typeOf<OnState>());
        EQ(0, ToOnFromOffGuardDummy::calls);
        EQ(0, ToOnFromOffActionSpy::calls());
        EQ(0, ToOffFromOnGuardDummy::calls);
        EQ(0, ToOffFromOnActionSpy::calls());
        EQ(0, ToOnFromOnGuardDummy::calls);
        EQ(0, ToOnFromOnActionSpy::calls());
        EQ(0, ToOffFromOffGuardDummy::calls);
        EQ(0, ToOffFromOffActionSpy::calls());

        // Off <- On, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::Off>();
        TRUE(result.activeState->typeOf<OffState>());
        EQ(0, ToOnFromOffGuardDummy::calls);
        EQ(0, ToOnFromOffActionSpy::calls());
        EQ(1, ToOffFromOnGuardDummy::calls);
        EQ(1, ToOffFromOnActionSpy::callsWithEvent);
        EQ(0, ToOnFromOnGuardDummy::calls);
        EQ(0, ToOnFromOnActionSpy::calls());
        EQ(0, ToOffFromOffGuardDummy::calls);
        EQ(0, ToOffFromOffActionSpy::calls());

        // Final <- Off, final transitions do not have and action or guard
        reset();
        sm.end();
        EQ(0, ToFinalFromOffGuardDummy::calls);
        EQ(0, ToFinalFromOffActionSpy::calls());
      }

      TEST_METHOD(ExplicitFinal)
      {
        using namespace StatemachineOnOffGuardAndActionTestImpl;
        OnState on;
        OffState off;

        Sm sm;
        sm.begin();

        // Final <- Off, final transitions do not have and action or guard
        reset();
        sm.dispatch<Trigger::Stop>();
        EQ(1, ToFinalFromOffGuardDummy::calls);
        EQ(1, ToFinalFromOffActionSpy::callsWithEvent);
      }

      void reset() const {
        using namespace StatemachineOnOffGuardAndActionTestImpl;
        ToOnFromOffGuardDummy::calls = 0;
        ToOnFromOffActionSpy::reset();
        ToOffFromOnGuardDummy::calls = 0;
        ToOffFromOnActionSpy::reset();
        ToOnFromOnGuardDummy::calls = 0;
        ToOnFromOnActionSpy::reset();
        ToOffFromOffGuardDummy::calls = 0;
        ToOffFromOffActionSpy::reset();
        ToFinalFromOffGuardDummy::calls = 0;
        ToFinalFromOffActionSpy::reset();
      }

    END

  }
}
