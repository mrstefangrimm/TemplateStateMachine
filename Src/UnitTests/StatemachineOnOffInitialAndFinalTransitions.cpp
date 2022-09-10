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

  namespace TransitionInitFinal {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace std;
    using namespace Helpers;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;
    typedef FactorCreator<StateType, false> StateTypeCreationPolicyType;

    enum Triggers {
      On,
      Off,
      Goodbye,
    };

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

    typedef GuardDummy<StateType, EmptyState<StateType>, OffState> ToFinalFromOffGuardDummy;
    typedef GuardDummy<StateType, EmptyState<StateType>, OnState> ToFinalFromOnGuardDummy;
    typedef ActionSpy<EmptyState<StateType>, struct OffState> ToFinalFromOffActionSpy;
    typedef ActionSpy<EmptyState<StateType>, struct OnState> ToFinalFromOnActionSpy;
    typedef Transition<Triggers::On, OnState, OffState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, OffState, OnState, StateTypeCreationPolicyType, OkGuard, EmptyAction> ToOffFromOnTransition;
    typedef FinalTransition<Triggers::Goodbye, OffState, StateTypeCreationPolicyType, ToFinalFromOffGuardDummy, ToFinalFromOffActionSpy> ToFinalFromOffTransition;
    typedef FinalTransition<Triggers::Goodbye, OnState, StateTypeCreationPolicyType, ToFinalFromOnGuardDummy, ToFinalFromOnActionSpy> ToFinalFromOnTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToFinalFromOffTransition,
      Typelist<ToFinalFromOnTransition,
      NullType>>>> TransitionList;

    typedef ActionSpy<struct OffState, struct EmptyState<StateType>> ToInitActionSpy;
    typedef InitialTransition<OffState, StateTypeCreationPolicyType, ToInitActionSpy> ToInitTransition;

    typedef GuardDummy<StateType, EmptyState<StateType>, AnyState<StateType>> ToEndFromAnyGuardDummy;
    typedef ActionSpy<struct EmptyState<StateType>, struct AnyState<StateType>> ToEndFromAnyActionSpy;
    typedef EndTransition<StateTypeCreationPolicyType, ToEndFromAnyGuardDummy, ToEndFromAnyActionSpy> ToEndTransition;
    
    typedef Statemachine<
      TransitionList,
      ToInitTransition,
      ToEndTransition> Sm;

    TEST_CLASS(StatemachineOnOffInitialAndFinalTransitions)
    {
    public:

      TEST_METHOD(InitialTransition_ToOffState_ActionEntryDoAreCalledAndFinalizeIsIgnored)
      {
        Sm sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
        ToEndFromAnyActionSpy::Calls = 0;

        auto result =sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
      }

      TEST_METHOD(InitialTransition_Fails_InitialTransitionDefinedWithGuard)
      {
        typedef GuardDummy<StateType, EmptyState<StateType>, AnyState<StateType>> ToOffFromInitGuardDummy;
        typedef ActionSpy<struct EmptyState<StateType>, struct AnyState<StateType>> ToOffFromInitActionSpy;
        // This is wrong: Use InitialTransition to avoid that "begin" fails.
        typedef Transition<0, OffState, OffState, StateTypeCreationPolicyType, ToOffFromInitGuardDummy, ToOffFromInitActionSpy> WrongToInitTransition;

        typedef Statemachine<
          TransitionList,
          WrongToInitTransition,
          ToEndTransition> SmWrongInitialTransition;

        SmWrongInitialTransition sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = true;
        ToFinalFromOffGuardDummy::CheckReturnValue = true;
        ToEndFromAnyActionSpy::Calls = 0;
        ToOffFromInitGuardDummy::Calls = 0;
        ToOffFromInitGuardDummy::CheckReturnValue = false;
        ToOffFromInitActionSpy::Calls = 0;

        auto result = sm.begin();
        Assert::AreEqual<bool>(false, result.consumed);
        Assert::IsNull<StateType>(result.activeState);
        // Guard and action are not even called when the active state is null.
        Assert::AreEqual<int>(0, ToOffFromInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOff_ExitAndActionAreCalled)
      {
        Sm sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
        ToFinalFromOnGuardDummy::CheckReturnValue = false;
        ToEndFromAnyActionSpy::Calls = 0;

        auto result =sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        ToEndFromAnyActionSpy::Calls = 0;

        ToFinalFromOffGuardDummy::CheckReturnValue = true;

        sm.dispatch<Triggers::Goodbye>();
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(1, ToFinalFromOffActionSpy::Calls, WSTRING("1, ToFinalFromOffActionSpy::Calls"));
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(1, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOffButGuardBlocks_ExitAndActionAreNotCalled)
      {
        Sm sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
        ToFinalFromOnGuardDummy::CheckReturnValue = false;
        ToEndFromAnyActionSpy::Calls = 0;

        sm.begin();
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);

        sm.dispatch<Triggers::Goodbye>();
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls, WSTRING("1, ToFinalFromOffActionSpy::Calls"));
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(1, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOn_ExitAndActionAreCalled)
      {
        Sm sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = false;
        ToFinalFromOnGuardDummy::CheckReturnValue = false;
        ToEndFromAnyActionSpy::Calls = 0;

        auto result =sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);

        // On <- Off
        sm.dispatch<Triggers::On>();
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);

        ToFinalFromOnGuardDummy::CheckReturnValue = true;

        sm.dispatch<Triggers::Goodbye>();
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(1, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(1, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(1, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
      }

      TEST_METHOD(End_FromStateOffAndEndTransitionDefined_ImplicitStateExitIsCalled)
      {
        Sm sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = true;
        ToFinalFromOnGuardDummy::CheckReturnValue = true;
        ToEndFromAnyActionSpy::Calls = 0;
        ToEndFromAnyGuardDummy::Calls = 0;
        ToEndFromAnyGuardDummy::CheckReturnValue = true;

        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);

        result = sm.end();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNull<StateType>(result.activeState);
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(1, ToEndFromAnyActionSpy::Calls);
      }

      TEST_METHOD(End_FromStateOnAndEndTransitionDefined_ImplicitStateExitIsCalled)
      {
        Sm sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = true;
        ToFinalFromOnGuardDummy::CheckReturnValue = true;
        ToEndFromAnyActionSpy::Calls = 0;
        ToEndFromAnyGuardDummy::Calls = 0;
        ToEndFromAnyGuardDummy::CheckReturnValue = true;

        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);

        // On <- Off
        sm.dispatch<Triggers::On>();
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);

        result = sm.end();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNull<StateType>(result.activeState);
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(1, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(1, ToEndFromAnyActionSpy::Calls);
      }

      TEST_METHOD(End_FromStateOffAndEndTransitionDefinedAndGuardBlocks_ActiveStateRemainsActive)
      {
        Sm sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToInitActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        ToFinalFromOffGuardDummy::Calls = 0;
        ToFinalFromOnGuardDummy::Calls = 0;
        ToFinalFromOffGuardDummy::CheckReturnValue = true;
        ToFinalFromOnGuardDummy::CheckReturnValue = true;
        ToEndFromAnyActionSpy::Calls = 0;
        ToEndFromAnyGuardDummy::Calls = 0;
        ToEndFromAnyGuardDummy::CheckReturnValue = true;

        auto result = sm.begin();
        Assert::AreEqual<bool>(true, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);

        ToEndFromAnyGuardDummy::CheckReturnValue = false;

        result = sm.end();
        Assert::AreEqual<bool>(false, result.consumed);
        Assert::IsNotNull<StateType>(result.activeState);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToInitActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnGuardDummy::Calls);
        Assert::AreEqual<int>(1, ToEndFromAnyGuardDummy::Calls);
        Assert::AreEqual<int>(0, ToEndFromAnyActionSpy::Calls);
      }

    };
  }
}
