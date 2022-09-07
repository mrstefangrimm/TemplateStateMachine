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

using namespace std;
#include "tsmlib/transition.h"
#include "tsmlib/statemachine.h"
#include "tsmlib/state.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace tsmlib;

namespace UnitTests {

  namespace TriggerExecutor {

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;

    enum Triggers {
      On,
      Off,
      Timeout,
      Wrong,
    };

    struct OnState : StateType, FactorCreator<OnState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 1; }
      void entry() { EntryCalls++; }
      void exit() { ExitCalls++; }
      template<uint8_t TRIGGER>
      void doit() { DoitCalls++; }
    };
    int OnState::EntryCalls = 0;
    int OnState::ExitCalls = 0;
    int OnState::DoitCalls = 0;

    struct OffState : StateType, FactorCreator<OffState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 2; }
      void entry() { EntryCalls++; }
      void exit() { ExitCalls++; }
      template<uint8_t TRIGGER>
      void doit() { DoitCalls++; }
    };
    int OffState::EntryCalls = 0;
    int OffState::ExitCalls = 0;
    int OffState::DoitCalls = 0;

    struct WrongState : StateType, FactorCreator<OnState> {
      uint8_t getTypeId() const override { return 3; }
    };

    typedef Transition<Triggers::On, StateType, OnState, OffState, EmptyGuard, EmptyAction> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, StateType, OffState, OnState, EmptyGuard, EmptyAction> ToOffFromOnTransition;
    typedef Transition<Triggers::Timeout, StateType, OnState, OnState, EmptyGuard, EmptyAction> ToOnFromOnTransition;
    typedef Transition<Triggers::Timeout, StateType, OffState, OffState, EmptyGuard, EmptyAction> ToOffFromOffTransition;

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
        typedef InitialTransition<StateType, OnState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OnState on;

        const int size = Length<TransitionList>::value;
        StateType* state = SM::TriggerExecutor<TransitionList, size - 1, Triggers::Timeout, StateType>().execute(&on);
        Assert::AreEqual<int>(on.getTypeId(), state->getTypeId());

        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOnTriggerOn_UnhandledTransformation)
      {
        typedef InitialTransition<StateType, OnState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OnState on;

        const int size = Length<TransitionList>::value;
        StateType* state = SM::TriggerExecutor<TransitionList, size - 1, Triggers::On, StateType>().execute(&on);
        Assert::IsTrue(0 == state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_WrongActiveStateTriggerTimeout_UnhandledTransformation)
      {
        typedef InitialTransition<StateType, OnState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        WrongState wrongState;

        const int size = Length<TransitionList>::value;
        StateType* state = SM::TriggerExecutor<TransitionList, size - 1, Triggers::Timeout, StateType>().execute(&wrongState);
        Assert::IsTrue(0 == state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOnTriggerWrong_UnhandledTransformation)
      {
        typedef InitialTransition<StateType, OnState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OnState on;

        const int size = Length<TransitionList>::value;
        StateType* state = SM::TriggerExecutor<TransitionList, size - 1, Triggers::Wrong, StateType>().execute(&on);
        Assert::IsTrue(0 == state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOffTriggerTimeout_TriggersOffStateDoit)
      {
        typedef InitialTransition<StateType, OffState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OffState off;

        const int size = Length<TransitionList>::value;
        StateType* state = SM::TriggerExecutor<TransitionList, size - 1, Triggers::Timeout, StateType>().execute(&off);
        Assert::AreEqual<int>(off.getTypeId(), state->getTypeId());

        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOffTriggerOff_UnhandledTransformation)
      {
        typedef InitialTransition<StateType, OnState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

        OffState::DoitCalls = 0;
        OnState::DoitCalls = 0;

        OffState off;

        const int size = Length<TransitionList>::value;
        StateType* state = SM::TriggerExecutor<TransitionList, size - 1, Triggers::Off, StateType>().execute(&off);
        Assert::IsTrue(0 == state);

        Assert::AreEqual<int>(0, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
      }

      TEST_METHOD(StatemachineTrigger_OnStateTimeout_TriggersOnStateDoit)
      {
        typedef InitialTransition<StateType, OnState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

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

        SM sm(true);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(0, OffState::EntryCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);

        sm.trigger<Triggers::Timeout>();
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(2, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(0, OffState::EntryCalls);
        Assert::AreEqual<int>(0, OffState::DoitCalls);
      }

      TEST_METHOD(StatemachineTrigger_OffStateTimeout_TriggersOffStateDoit)
      {
        typedef InitialTransition<StateType, OffState, EmptyAction> InitTransition;
        typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

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

        SM sm(true);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);

        sm.trigger<Triggers::Timeout>();
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
