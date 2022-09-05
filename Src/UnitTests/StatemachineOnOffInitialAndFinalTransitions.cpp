#include "pch.h"
#include "CppUnitTest.h"

#include "tsmlib/state.h"
#include "tsmlib/templatemeta.h"
#include "tsmlib/statemachine.h"
#include "tsmlib/transition.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace tsmlib;
using namespace std;

namespace unittests_tsm_initialfinal {

  typedef State<VirtualGetTypeIdStateComperator, false> StateType;

  template<typename TO, typename FROM>
  struct ActionSpy {
    static int Calls;
    void operator()() { Calls++; }
  };
  template<typename TO, typename FROM> int ActionSpy<TO, FROM>::Calls = 0;

  typedef ActionSpy<struct OffState, struct EmptyState<StateType>> ToOffFromInitialActionSpy;
  typedef ActionSpy<EmptyState<StateType>, struct OffState> ToFinalFromOffActionSpy;
  typedef ActionSpy<EmptyState<StateType>, struct OnState> ToFinalFromOnActionSpy;

  struct TerminateActionSpy {
    static int Calls;
    void operator()() { Calls++; }
  };
  int TerminateActionSpy::Calls = 0;

  struct TerminateGuardDummy {
    static int Calls;
    static bool CheckReturnValue;
    bool check() { Calls++; return CheckReturnValue; }
  };
  int TerminateGuardDummy::Calls = 0;
  bool TerminateGuardDummy::CheckReturnValue = false;

  enum Triggers {
    On,
    Off,
    GoodbyeOff,
    GoodbyeOn
  };

  struct OnState : StateType, FactorCreator<OnState> {
    static int EntryCalls;
    static int ExitCalls;
    static int DoCalls;

    uint8_t getTypeId() const override { return 1; }
    void entry() { EntryCalls++; }
    void exit() { ExitCalls++; }
    void doit() { DoCalls++; }
  };
  int OnState::EntryCalls = 0;
  int OnState::ExitCalls = 0;
  int OnState::DoCalls = 0;

  struct OffState : StateType, FactorCreator<OffState> {
    static int EntryCalls;
    static int ExitCalls;
    static int DoCalls;
    
    uint8_t getTypeId() const override { return 2; }
    void entry() { EntryCalls++; }
    void exit() { ExitCalls++; }
    void doit() { DoCalls++; }
  };
  int OffState::EntryCalls = 0;
  int OffState::ExitCalls = 0;
  int OffState::DoCalls = 0;

  typedef Transition<Triggers::On, StateType, OnState, OffState, EmptyGuard, EmptyAction> ToOnFromOffTransition;
  typedef Transition<Triggers::Off, StateType, OffState, OnState, EmptyGuard, EmptyAction> ToOffFromOnTransition;
  typedef Transition<Triggers::GoodbyeOff, StateType, EmptyState<StateType>, OffState, EmptyGuard, ToFinalFromOffActionSpy> ToFinalFromOffTransition;
  typedef Transition<Triggers::GoodbyeOn, StateType, EmptyState<StateType>, OnState, EmptyGuard, ToFinalFromOnActionSpy> ToFinalffFromOnTransition;

  typedef
    Typelist<ToOnFromOffTransition,
    Typelist<ToOffFromOnTransition,
    Typelist<ToFinalFromOffTransition,
    Typelist<ToFinalffFromOnTransition,
    NullType>>>> TransitionList;

  typedef InitialTransition<StateType, OffState, ToOffFromInitialActionSpy> InitTransition;
  typedef FinalTransition<StateType, TerminateGuardDummy, TerminateActionSpy> TerminateTransition;
  typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, TerminateTransition> SM;

  TEST_CLASS(StatemachineOnOffInitialAndFinalTransitions)
  {
  public:

    TEST_METHOD(InitialTransition_ToOffState_ActionEntryDoAreCalled)
    {
      OffState::ExitCalls = 0;
      OffState::EntryCalls = 0;
      OffState::DoCalls = 0;
      OnState::ExitCalls = 0;
      OnState::EntryCalls = 0;
      OnState::DoCalls = 0;
      ToOffFromInitialActionSpy::Calls = 0;
      ToFinalFromOffActionSpy::Calls = 0;
      ToFinalFromOnActionSpy::Calls = 0;
      TerminateActionSpy::Calls = 0;
      TerminateGuardDummy::Calls = 0;
      TerminateGuardDummy::CheckReturnValue = false;

      SM sm;
      Assert::AreEqual<int>(0, OffState::ExitCalls);
      Assert::AreEqual<int>(1, OffState::EntryCalls);
      Assert::AreEqual<int>(1, OffState::DoCalls);
      Assert::AreEqual<int>(0, OnState::ExitCalls);
      Assert::AreEqual<int>(0, OnState::EntryCalls);
      Assert::AreEqual<int>(0, OnState::DoCalls);
      Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateGuardDummy::Calls);
    }

    // TODO
    // Assert::AreEqual<int>(1, OffState::ExitCalls); fails
    // State would need a virtual bool Exit() = 0; method, otherwise the _activeState in Statemachine cannot select the derived implementation.
    //TEST_METHOD(ImplicitFinializeTransition_ToOnFromOffAndFinalizeCondition)
    //{
    //  TerminateGuardDummy::CheckReturnValue = true;
    //  SM sm;
    //  sm.Trigger<1>();
    //}

    TEST_METHOD(ExplicitFinializeTransition_FromOff_OffExitAndActionAreCalled)
    {
      OffState::ExitCalls = 0;
      OffState::EntryCalls = 0;
      OffState::DoCalls = 0;
      OnState::ExitCalls = 0;
      OnState::EntryCalls = 0;
      OnState::DoCalls = 0;
      ToOffFromInitialActionSpy::Calls = 0;
      ToFinalFromOffActionSpy::Calls = 0;
      ToFinalFromOnActionSpy::Calls = 0;
      TerminateActionSpy::Calls = 0;
      TerminateGuardDummy::Calls = 0;
      TerminateGuardDummy::CheckReturnValue = true;

      SM sm;
      Assert::AreEqual<int>(0, OffState::ExitCalls);
      Assert::AreEqual<int>(1, OffState::EntryCalls);
      Assert::AreEqual<int>(1, OffState::DoCalls);
      Assert::AreEqual<int>(0, OnState::ExitCalls);
      Assert::AreEqual<int>(0, OnState::EntryCalls);
      Assert::AreEqual<int>(0, OnState::DoCalls);
      Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateGuardDummy::Calls);

      sm.trigger<Triggers::GoodbyeOff>();
      Assert::AreEqual<int>(1, OffState::ExitCalls);
      Assert::AreEqual<int>(1, OffState::EntryCalls);
      Assert::AreEqual<int>(1, OffState::DoCalls);
      Assert::AreEqual<int>(0, OnState::ExitCalls);
      Assert::AreEqual<int>(0, OnState::EntryCalls);
      Assert::AreEqual<int>(0, OnState::DoCalls);
      Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
      Assert::AreEqual<int>(1, ToFinalFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateGuardDummy::Calls);
    }

    TEST_METHOD(ExplicitFinializeTransition_FromOn_OnExitAndActionAreCalled)
    {
      OffState::ExitCalls = 0;
      OffState::EntryCalls = 0;
      OffState::DoCalls = 0;
      OnState::ExitCalls = 0;
      OnState::EntryCalls = 0;
      OnState::DoCalls = 0;
      ToOffFromInitialActionSpy::Calls = 0;
      ToFinalFromOffActionSpy::Calls = 0;
      ToFinalFromOnActionSpy::Calls = 0;
      TerminateActionSpy::Calls = 0;
      TerminateGuardDummy::Calls = 0;
      TerminateGuardDummy::CheckReturnValue = true;

      SM sm;
      Assert::AreEqual<int>(0, OffState::ExitCalls);
      Assert::AreEqual<int>(1, OffState::EntryCalls);
      Assert::AreEqual<int>(1, OffState::DoCalls);
      Assert::AreEqual<int>(0, OnState::ExitCalls);
      Assert::AreEqual<int>(0, OnState::EntryCalls);
      Assert::AreEqual<int>(0, OnState::DoCalls);
      Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateGuardDummy::Calls);

      // On <- Off
      sm.trigger<Triggers::On>();
      Assert::AreEqual<int>(1, OffState::ExitCalls);
      Assert::AreEqual<int>(1, OffState::EntryCalls);
      Assert::AreEqual<int>(1, OffState::DoCalls);
      Assert::AreEqual<int>(0, OnState::ExitCalls);
      Assert::AreEqual<int>(1, OnState::EntryCalls);
      Assert::AreEqual<int>(1, OnState::DoCalls);
      Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateGuardDummy::Calls);

      sm.trigger<Triggers::GoodbyeOn>();
      Assert::AreEqual<int>(1, OffState::ExitCalls);
      Assert::AreEqual<int>(1, OffState::EntryCalls);
      Assert::AreEqual<int>(1, OffState::DoCalls);
      Assert::AreEqual<int>(1, OnState::ExitCalls);
      Assert::AreEqual<int>(1, OnState::EntryCalls);
      Assert::AreEqual<int>(1, OnState::DoCalls);
      Assert::AreEqual<int>(1, ToOffFromInitialActionSpy::Calls);
      Assert::AreEqual<int>(0, ToFinalFromOffActionSpy::Calls);
      Assert::AreEqual<int>(1, ToFinalFromOnActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateActionSpy::Calls);
      Assert::AreEqual<int>(0, TerminateGuardDummy::Calls);
    }

  };
}
