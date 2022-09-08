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

#include "tsmlib/state.h"
#include "tsmlib/templatemeta.h"
#include "tsmlib/statemachine.h"
#include "tsmlib/transition.h"

#define CAT(A, B) A##B
#define WSTRING(A) CAT(L, #A)

namespace UnitTests {

  namespace TransitionInitFinal {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace std;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;

    template<typename TO, typename FROM>
    struct ActionSpy {
      static int Calls;
      template<typename T>
      void perform(T*) { Calls++; }
    };
    template<typename TO, typename FROM> int ActionSpy<TO, FROM>::Calls = 0;

    typedef ActionSpy<struct OffState, struct EmptyState<StateType>> ToOffFromInitialActionSpy;
    typedef ActionSpy<EmptyState<StateType>, struct OffState> ToFinalFromOffActionSpy;
    typedef ActionSpy<EmptyState<StateType>, struct OnState> ToFinalFromOnActionSpy;

    struct TerminateActionSpy {
      static int Calls;
      template<typename T>
      void perform(T*) { Calls++; }
    };
    int TerminateActionSpy::Calls = 0;

    struct FinalFromOffGuardDummy {
      static int Calls;
      static bool CheckReturnValue;
      template<typename T>
      bool check(T*) { Calls++; return CheckReturnValue; }
    };
    int FinalFromOffGuardDummy::Calls = 0;
    bool FinalFromOffGuardDummy::CheckReturnValue = false;

    struct FinalFromOnGuardDummy {
      static int Calls;
      static bool CheckReturnValue;
      template<typename T>
      bool check(T*) { Calls++; return CheckReturnValue; }
    };
    int FinalFromOnGuardDummy::Calls = 0;
    bool FinalFromOnGuardDummy::CheckReturnValue = false;

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

    typedef Transition<Triggers::On, StateType, OnState, OffState, OkGuard, EmptyAction, false> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, StateType, OffState, OnState, OkGuard, EmptyAction, false> ToOffFromOnTransition;
    typedef Transition<Triggers::Goodbye, StateType, EmptyState<StateType>, OffState, FinalFromOffGuardDummy, ToFinalFromOffActionSpy, false> ToFinalFromOffTransition;
    typedef Transition<Triggers::Goodbye, StateType, EmptyState<StateType>, OnState, FinalFromOnGuardDummy, ToFinalFromOnActionSpy, false> ToFinalffFromOnTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToFinalFromOffTransition,
      Typelist<ToFinalffFromOnTransition,
      NullType>>>> TransitionList;

    typedef InitialTransition<StateType, OffState, ToOffFromInitialActionSpy> InitTransition;
    typedef Statemachine<StateType, TransitionList, InitTransition, NullFinalTransition<StateType>> SM;

    TEST_CLASS(StatemachineOnOffInitialAndFinalTransitions)
    {
    public:

      TEST_METHOD(InitialTransition_ToOffState_ActionEntryDoAreCalledAndFinalizeIsIgnored)
      {
        SM sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToOffFromInitialActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        TerminateActionSpy::Calls = 0;
        FinalFromOffGuardDummy::Calls = 0;
        FinalFromOnGuardDummy::Calls = 0;
        FinalFromOffGuardDummy::CheckReturnValue = true;
        FinalFromOffGuardDummy::CheckReturnValue = false;

        sm.begin();
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOff_ExitAndActionAreCalled)
      {
        SM sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToOffFromInitialActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        TerminateActionSpy::Calls = 0;
        FinalFromOffGuardDummy::Calls = 0;
        FinalFromOnGuardDummy::Calls = 0;
        FinalFromOffGuardDummy::CheckReturnValue = false;
        FinalFromOnGuardDummy::CheckReturnValue = false;

        sm.begin();
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(0, FinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, FinalFromOnGuardDummy::Calls);

        FinalFromOffGuardDummy::CheckReturnValue = true;

        sm.trigger<Triggers::Goodbye>();
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(1, ToFinalFromOffActionSpy::Calls, WSTRING("1, ToFinalFromOffActionSpy::Calls"));
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(1, FinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, FinalFromOnGuardDummy::Calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOffButGuardBlocks_ExitAndActionAreNotCalled)
      {
        SM sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToOffFromInitialActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        TerminateActionSpy::Calls = 0;
        FinalFromOffGuardDummy::Calls = 0;
        FinalFromOnGuardDummy::Calls = 0;
        FinalFromOffGuardDummy::CheckReturnValue = false;
        FinalFromOnGuardDummy::CheckReturnValue = false;

        sm.begin();
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(0, FinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, FinalFromOnGuardDummy::Calls);

        sm.trigger<Triggers::Goodbye>();
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls, WSTRING("1, ToFinalFromOffActionSpy::Calls"));
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(1, FinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, FinalFromOnGuardDummy::Calls);
      }

      TEST_METHOD(ExplicitFinializeTransition_FromStateOn_ExitAndActionAreCalled)
      {
        SM sm;
        OffState::ExitCalls = 0;
        OffState::EntryCalls = 0;
        OffState::DoitCalls = 0;
        OnState::ExitCalls = 0;
        OnState::EntryCalls = 0;
        OnState::DoitCalls = 0;
        ToOffFromInitialActionSpy::Calls = 0;
        ToFinalFromOffActionSpy::Calls = 0;
        ToFinalFromOnActionSpy::Calls = 0;
        TerminateActionSpy::Calls = 0;
        FinalFromOffGuardDummy::Calls = 0;
        FinalFromOnGuardDummy::Calls = 0;
        FinalFromOffGuardDummy::CheckReturnValue = false;
        FinalFromOnGuardDummy::CheckReturnValue = false;

        sm.begin();
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(0, FinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, FinalFromOnGuardDummy::Calls);

        // On <- Off
        sm.trigger<Triggers::On>();
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(0, FinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(0, FinalFromOnGuardDummy::Calls);

        FinalFromOnGuardDummy::CheckReturnValue = true;

        sm.trigger<Triggers::Goodbye>();
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(1, OffState::DoitCalls);
        Assert::AreEqual<int>(1, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
        Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
        Assert::AreEqual<int>(1, ToFinalFromOnActionSpy::Calls);
        Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
        Assert::AreEqual<int>(0, FinalFromOffGuardDummy::Calls);
        Assert::AreEqual<int>(1, FinalFromOnGuardDummy::Calls);
      }

    };
  }
}
