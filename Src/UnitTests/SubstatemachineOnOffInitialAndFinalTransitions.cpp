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

#include "tsmlib/state.h"
#include "tsmlib/templatemeta.h"
#include "tsmlib/statemachine.h"
#include "tsmlib/transition.h"
#include "TestHelpers.h"

#define CAT(A, B) A##B
#define WSTRING(A) CAT(L, #A)

namespace UnitTests {

  namespace SubstatesTransitionInitFinal {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace std;
    using namespace Helpers;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;
    typedef FactorCreator<StateType, false> StateTypeCreationPolicyType;

    typedef ActionSpy<struct OffState, struct EmptyState<StateType>> ToOffFromInitialActionSpy;
    typedef ActionSpy<EmptyState<StateType>, struct OffState> ToFinalFromOffActionSpy;
    typedef ActionSpy<EmptyState<StateType>, struct OnState> ToFinalFromOnActionSpy;

    struct TerminateActionSpy {
      static int Calls;
      template<typename T>
      void perform(T*) { Calls++; }
    };
    int TerminateActionSpy::Calls = 0;

    enum Triggers {
      On,
      Off,
      Goodbye,
      GoodbyeSub,
      Hello,
    };

    struct IdleState : SimpleState<IdleState, StateType>, FactorCreator<IdleState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 1; }

    private:
      friend class SimpleState<IdleState, StateType>;
      void entry_() { EntryCalls++; }
      void exit_() { ExitCalls++; }
      template<uint8_t N>
      void doit_() { DoitCalls++; }
    };
    int IdleState::EntryCalls = 0;
    int IdleState::ExitCalls = 0;
    int IdleState::DoitCalls = 0;

    struct OnState : SimpleState<OnState, StateType>, FactorCreator<OnState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 1; }

    private:
      friend class SimpleState<OnState, StateType>;
      void entry_() { EntryCalls++; }
      void exit_() { ExitCalls++; }
      template<uint8_t N>
      void doit_() { DoitCalls++; }
    };
    int OnState::EntryCalls = 0;
    int OnState::ExitCalls = 0;
    int OnState::DoitCalls = 0;

    struct OffState : SimpleState<OffState, StateType>, FactorCreator<OffState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 2; }

    private:
      friend class SimpleState<OffState, StateType>;
      void entry_() { EntryCalls++; }
      void exit_() { ExitCalls++; }
      template<uint8_t N>
      void doit_() { DoitCalls++; }
    };
    int OffState::EntryCalls = 0;
    int OffState::ExitCalls = 0;
    int OffState::DoitCalls = 0;

    typedef GuardDummy<StateType, AnyState<StateType>, AnyState<StateType>> ToFinalSubOnGuardDummy;
    typedef ActionSpy<AnyState<StateType>, AnyState<StateType>> ToFinalSubOnActionSpy;

    typedef Transition<Triggers::On, OnState, OffState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, OffState, OnState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOffFromOnTransition;
    typedef ExitTransition<Triggers::GoodbyeSub, IdleState, OnState, StateTypeCreationPolicyType, ToFinalSubOnGuardDummy, ToFinalSubOnActionSpy> ToIdleFromOnTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToIdleFromOnTransition,
      NullType>>> ActivestateTransitionList;

    typedef InitialTransition<OffState, StateTypeCreationPolicyType, EmptyAction> ActivestateInitTransition;
    typedef Statemachine<
      ActivestateTransitionList,
      ActivestateInitTransition,
      NullEndTransition<StateTypeCreationPolicyType>> ActivestateStatemachine;

    struct ActiveState : SubstatesHolderState<ActiveState, StateType, ActivestateStatemachine>, FactorCreator<ActiveState> {
      static int EntryCalls;
      static int ExitCalls;
      static int DoitCalls;

      uint8_t getTypeId() const override { return 3; }

    private:
      friend class SubstatesHolderState<ActiveState, StateType, ActivestateStatemachine>;
      void entry_() { EntryCalls++; }
      void exit_() { ExitCalls++; }
      template<uint8_t N>
      void doit_() { DoitCalls++; }
    };
    int ActiveState::EntryCalls = 0;
    int ActiveState::ExitCalls = 0;
    int ActiveState::DoitCalls = 0;

    typedef GuardDummy<StateType, AnyState<StateType>, AnyState<StateType>> ToFinalSubstatesGuardDummy;
    typedef ActionSpy<AnyState<StateType>, AnyState<StateType>> ToFinalSubstatesActionSpy;

    // sub-states transitions are self transitions
    typedef Declaration<Triggers::On, ActiveState, StateTypeCreationPolicyType> ToOnFromOffSubTransition;
    typedef Declaration<Triggers::Off, ActiveState, StateTypeCreationPolicyType> ToOffFromOnSubTransition;
    typedef ExitDeclaration<Triggers::GoodbyeSub, IdleState, ActiveState, StateTypeCreationPolicyType> ToIdleFromOffSubTransition;

    typedef Transition<Triggers::Hello, ActiveState, IdleState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToActiveFromIdleTransition;
    typedef Transition<Triggers::Goodbye, IdleState, AnyState<StateType>, StateTypeCreationPolicyType, ToFinalSubstatesGuardDummy, ToFinalSubstatesActionSpy> ToIdleFromActiveTransition;

    typedef
      Typelist<ToOnFromOffSubTransition,
      Typelist<ToOffFromOnSubTransition,
      Typelist<ToIdleFromOffSubTransition,
      Typelist<ToActiveFromIdleTransition,
      Typelist<ToIdleFromActiveTransition,
      NullType>>>>> TransitionList;

    struct ActiveStateFinalizeGuard {
      template<typename T>
      bool check(T*) { return true; }
    };

    typedef InitialTransition<IdleState, StateTypeCreationPolicyType, EmptyAction> InitTransition;
    typedef EndTransition<StateTypeCreationPolicyType, ActiveStateFinalizeGuard, EmptyAction> ActivestateFinalTransition;
    typedef Statemachine<
      TransitionList,
      InitTransition,
      ActivestateFinalTransition> Sm;

    TEST_CLASS(SubstatemachineOnOffInitialAndFinalTransitions)
    {
    public:

      TEST_METHOD(ExitfromActiveExitsSubstate)
      {
        ToFinalSubstatesGuardDummy::Calls = 0;
        ToFinalSubstatesActionSpy::Calls = 0;
        ToFinalSubOnGuardDummy::Calls = 0;
        ToFinalSubOnActionSpy::Calls = 0;
        OnState::ExitCalls = 0;
        ActiveState::ExitCalls = 0;

        Sm sm;

        sm.begin();

        sm.dispatch<Triggers::Hello>();

        sm.dispatch<Triggers::On>();

        sm.dispatch<Triggers::Goodbye>();

        Assert::AreEqual<int>(1, ToFinalSubstatesGuardDummy::Calls);
        Assert::AreEqual<int>(1, ToFinalSubstatesActionSpy::Calls);
        Assert::AreEqual<int>(1, ToFinalSubOnGuardDummy::Calls);
        Assert::AreEqual<int>(1, ToFinalSubOnActionSpy::Calls);
        Assert::AreEqual<int>(1, OnState::ExitCalls);
        Assert::AreEqual<int>(1, ActiveState::ExitCalls);
      }

      TEST_METHOD(transitionfromsubstateout_subandsupperstateexitiscalled)
      {
        ToFinalSubstatesGuardDummy::Calls = 0;
        ToFinalSubstatesActionSpy::Calls = 0;
        ToFinalSubOnGuardDummy::Calls = 0;
        ToFinalSubOnActionSpy::Calls = 0;
        OnState::ExitCalls = 0;
        ActiveState::ExitCalls = 0;

        Sm sm;

        sm.begin();

        sm.dispatch<Triggers::Hello>();

        sm.dispatch<Triggers::On>();

        sm.dispatch<Triggers::GoodbyeSub>();

        //Assert::AreEqual<int>(1, ToFinalSubstatesGuardDummy::Calls);
        //Assert::AreEqual<int>(1, ToFinalSubstatesActionSpy::Calls);
        //Assert::AreEqual<int>(1, ToFinalSubOnGuardDummy::Calls);
        //Assert::AreEqual<int>(1, ToFinalSubOnActionSpy::Calls);
        Assert::AreEqual<int>(1, OnState::ExitCalls);
        Assert::AreEqual<int>(1, ActiveState::ExitCalls);
      }

    };
  }
}
