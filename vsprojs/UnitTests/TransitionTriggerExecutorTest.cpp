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

#include "CppUnitTest.h"

using namespace std;
#include "..\..\src\transition.h"
#include "..\..\src\statemachine.h"
#include "..\..\src\state.h"
#include "..\..\src\initialtransition.h"
#include "..\..\src\endtransition.h"

namespace UnitTests {

  namespace TriggerExecutor {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace Loki;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;
    typedef FactorCreator<StateType, false> StateTypeCreationPolicyType;

    enum Triggers {
      On,
      Off,
      Timeout,
      Wrong,
    };

    struct OnState : BasicState<OnState, StateType>, FactorCreator<OnState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 1; }

    private:
      friend class BasicState<OnState, StateType>;
      void entry() { EntryCalls++; }
      void exit() { ExitCalls++; }
      template<uint8_t N>
      void doit() { DoitCalls++; }
    };
    int OnState::EntryCalls = 0;
    int OnState::ExitCalls = 0;
    int OnState::DoitCalls = 0;

    struct OffState : BasicState<OffState, StateType>, FactorCreator<OffState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 2; }

    private:
      friend class BasicState<OffState, StateType>;
      void entry() { EntryCalls++; }
      void exit() { ExitCalls++; }
      template<uint8_t N>
      void doit() { DoitCalls++; }
    };
    int OffState::EntryCalls = 0;
    int OffState::ExitCalls = 0;
    int OffState::DoitCalls = 0;

    struct WrongState : StateType, FactorCreator<OnState> {
      uint8_t getTypeId() const override { return 3; }
      void _vexit() { }
    };

    typedef Transition<Triggers::On, OnState, OffState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, OffState, OnState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOffFromOnTransition;
    typedef SelfTransition<Triggers::Timeout, OnState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOnFromOnTransition;
    typedef SelfTransition<Triggers::Timeout, OffState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOffFromOffTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToOnFromOnTransition,
      Typelist<ToOffFromOffTransition,
      NullType>>>> TransitionList;

    TEST_CLASS(TransitionTriggerExecutorTest)
    {
    public:

      TEST_METHOD(Execute_ActiveStateOnTriggerTimeout_TriggersOnStateDoit)
      {
        typedef InitialTransition<OnState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OnState on;

        const int size = Length<TransitionList>::value;
        auto result = Sm::TriggerExecutor<Triggers::Timeout, size - 1>().execute(&on);
        Assert::AreEqual<int>(on.getTypeId(), result.state->getTypeId());

        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOnTriggerOn_UnhandledTransition)
      {
        typedef InitialTransition<OnState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OnState on;

        const int size = Length<TransitionList>::value;
        auto result = Sm::TriggerExecutor<Triggers::On, size - 1>().execute(&on);
        Assert::IsTrue(0 == result.state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_WrongActiveStateTriggerTimeout_UnhandledTransition)
      {
        typedef InitialTransition<OnState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        WrongState wrongState;

        const int size = Length<TransitionList>::value;
        auto result = Sm::TriggerExecutor<Triggers::Timeout, size - 1>().execute(&wrongState);
        Assert::IsTrue(0 ==  result.state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOnTriggerWrong_UnhandledTransition)
      {
        typedef InitialTransition<OnState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OnState on;

        const int size = Length<TransitionList>::value;
        auto result = Sm::TriggerExecutor<Triggers::Wrong, size - 1>().execute(&on);
        Assert::IsTrue(0 == result.state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOffTriggerTimeout_TriggersOffStateDoit)
      {
        typedef InitialTransition<OffState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OffState off;

        const int size = Length<TransitionList>::value;
        auto result = Sm::TriggerExecutor<Triggers::Timeout, size - 1>().execute(&off);
        Assert::AreEqual<int>(off.getTypeId(), result.state->getTypeId());

        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOffTriggerOff_UnhandledTransition)
      {
        typedef InitialTransition<OnState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OffState off;

        //const int size = Length<TransitionList>::value;
        //StateType* state = Sm::TriggerExecutor<Triggers::Off, size - 1>().execute(&off);
        //Assert::IsTrue(0 == state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(StatemachineTrigger_OnStateTimeout_TriggersOnStateDoit)
      {
        typedef InitialTransition<OnState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;

        OnState on;
        OffState off;
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(0, OffState::EntryCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);

        Sm sm;
        sm.begin();
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(0, OffState::EntryCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);

        sm.dispatch<Triggers::Timeout>();
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(2, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(0, OffState::EntryCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);
      }

      TEST_METHOD(StatemachineTrigger_OffStateTimeout_TriggersOffStateDoit)
      {
        typedef InitialTransition<OffState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
        typedef Statemachine<TransitionList, InitTransition, NullEndTransition<StateTypeCreationPolicyType>> Sm;

        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;

        OnState on;
        OffState off;
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(0, OffState::EntryCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);

        Sm sm;
        sm.begin();
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);

        sm.dispatch<Triggers::Timeout>();
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(2, OffState::DoitCalls);
      }
    };
  }
}
