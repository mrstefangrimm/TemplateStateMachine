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

namespace UT {
  namespace StatemachineOnOff {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace StatemachineOnOffGuardAndActionTestImpl {

      using StateType = State<VirtualGetTypeIdStateComparator, false>;
      using StateTypeCreationPolicyType = FactoryCreator<StateType, false>;

      using ToOnFromOffActionSpy = ActionStub<struct OnState, struct OffState>;
      using ToOffFromOnActionSpy = ActionStub<struct OffState, struct OnState>;
      using ToOnFromOnActionSpy = ActionStub<struct OnState, struct OnState>;
      using ToOffFromOffActionSpy = ActionStub<struct OffState, struct OffState>;

      using ToOnFromOffGuardDummy = GuardDummy<StateType, struct OnState, struct OffState>;
      using ToOffFromOnGuardDummy = GuardDummy<StateType, struct OffState, struct OnState>;
      using ToOnFromOnGuardDummy = GuardDummy<StateType, struct OnState, struct OnState>;
      using ToOffFromOffGuardDummy = GuardDummy<StateType, struct OffState, struct OffState>;

      namespace Trigger
      {
        struct On {};
        struct Off {};
        struct OnToOn {};
        struct OffToOff {};
      }

      struct OnState : BasicState<OnState, StateType>, FactoryCreator<OnState> {
        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<OnState, StateType>;
        template<class Event> void entry(const Event&) { }
        template<class Event> void exit(const Event&) { }
        template<class Event> void doit(const Event&) { }
      };

      struct OffState : BasicState<OffState, StateType>, FactoryCreator<OffState> {
        uint8_t getTypeId() const override { return 2; }

      private:
        friend class BasicState<OffState, StateType>;
        template<class Event> void entry(const Event&) { }
        template<class Event> void exit(const Event&) { }
        template<class Event> void doit(const Event&) { }
      };

      using ToOnFromOffTransition = Transition<Trigger::On, OnState, OffState, StateTypeCreationPolicyType, ToOnFromOffGuardDummy, ToOnFromOffActionSpy>;
      using ToOffFromOnTransition = Transition<Trigger::Off, OffState, OnState, StateTypeCreationPolicyType, ToOffFromOnGuardDummy, ToOffFromOnActionSpy>;
      using ToOnFromOnTransition = SelfTransition<Trigger::OnToOn, OnState, StateTypeCreationPolicyType, ToOnFromOnGuardDummy, ToOnFromOnActionSpy, false>;
      using ToOffFromOffTransition = SelfTransition<Trigger::OffToOff, OffState, StateTypeCreationPolicyType, ToOffFromOffGuardDummy, ToOffFromOffActionSpy, false>;

      using TransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToOnFromOnTransition,
        Typelist<ToOffFromOffTransition,
        NullType>>>>;

      using InitTransition = InitialTransition<OffState, StateTypeCreationPolicyType, NoAction>;
      using Sm = Statemachine<TransitionList, InitTransition>;
    }

    TEST_CLASS(StatemachineOnOffGuardAndActionTest)
    {
      TEST_METHOD_INITIALIZE(Initialize)
      {
        reset();
      }

      TEST_METHOD(GuardsAndActions_Roundtrip)
      {
        using namespace StatemachineOnOffGuardAndActionTestImpl;
        OnState on;
        OffState off;
 
        Sm sm;
        sm.begin();
        // Off <- Off, self transition
        auto result = sm.dispatch<Trigger::OffToOff>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnActionSpy::calls);
        Assert::AreEqual<int>(1, ToOffFromOffGuardDummy::calls);
        Assert::AreEqual<int>(1, ToOffFromOffActionSpy::calls);

        // Off <- Off, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::Off>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffActionSpy::calls);

        // Off <- Off, guard = false
        reset();
        ToOnFromOffGuardDummy::CheckReturnValue = false;
        result = sm.dispatch<Trigger::On>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(1, ToOnFromOffGuardDummy::calls);
        Assert::AreEqual<int>(1, ToOnFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffActionSpy::calls);

        // On <- Off
        reset();
        ToOnFromOffGuardDummy::CheckReturnValue = true;
        result = sm.dispatch<Trigger::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(1, ToOnFromOffGuardDummy::calls);
        Assert::AreEqual<int>(1, ToOnFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffActionSpy::calls);

        // On <- On, self transition
        reset();
        result = sm.dispatch<Trigger::OnToOn>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnActionSpy::calls);
        Assert::AreEqual<int>(1, ToOnFromOnGuardDummy::calls);
        Assert::AreEqual<int>(1, ToOnFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffActionSpy::calls);

        // On <- On, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOffActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffActionSpy::calls);

        // Off <- On, unhandled trigger
        reset();
        result = sm.dispatch<Trigger::Off>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, ToOnFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOffActionSpy::calls);
        Assert::AreEqual<int>(1, ToOffFromOnGuardDummy::calls);
        Assert::AreEqual<int>(1, ToOffFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOnFromOnActionSpy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffGuardDummy::calls);
        Assert::AreEqual<int>(0, ToOffFromOffActionSpy::calls);
      }

      private:
        void reset() const {
          using namespace StatemachineOnOffGuardAndActionTestImpl;
          ToOnFromOffGuardDummy::calls = 0;
          ToOnFromOffActionSpy::calls = 0;
          ToOffFromOnGuardDummy::calls = 0;
          ToOffFromOnActionSpy::calls = 0;
          ToOnFromOnGuardDummy::calls = 0;
          ToOnFromOnActionSpy::calls = 0;
          ToOffFromOffGuardDummy::calls = 0;
          ToOffFromOffActionSpy::calls = 0;
        }
    };
  }
}
