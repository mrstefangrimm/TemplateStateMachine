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

namespace UnitTests {

  namespace Lifecycle {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace std;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;

    enum Triggers {
      On,
      Off,
      OnToOn,
      OffToOff
    };

    template<typename T>
    struct FactorCreatorFake {
      typedef FactorCreatorFake<T> CreatorType;
      typedef T ObjectType;

      static int CreateCalls;
      static int DeleteCalls;

      typedef FactorCreatorFake<T> CreatorType;

      static T* create() { CreateCalls++;  return new T; }
      static void destroy(T* state) { DeleteCalls++;  delete state; }
    };
    template<typename T> int FactorCreatorFake<T>::CreateCalls = 0;
    template<typename T> int FactorCreatorFake<T>::DeleteCalls = 0;

    struct OnState : SimpleState<OnState, StateType>, FactorCreatorFake<OnState> {
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

    struct OffState : SimpleState<OffState, StateType>, FactorCreatorFake<OffState> {
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

    typedef Transition<Triggers::On, StateType, OnState, OffState, OkGuard, EmptyAction> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, StateType, OffState, OnState, OkGuard, EmptyAction> ToOffFromOnTransition;
    typedef SelfTransition<Triggers::OnToOn, StateType, OnState, OkGuard, EmptyAction> ToOnFromOnTransition;
    typedef SelfTransition<Triggers::OffToOff, StateType, OffState, OkGuard, EmptyAction> ToOffFromOffTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToOnFromOnTransition,
      Typelist<ToOffFromOffTransition,
      NullType>>>> TransitionList;

    typedef InitialTransition<StateType, OffState, EmptyAction> InitTransition;
    typedef Statemachine<StateType, TransitionList, InitTransition, NullFinalTransition<StateType>> Sm;

    TEST_CLASS(StatemachineOnOffEntryDoExitTest)
    {
    public:

      TEST_METHOD(EntriesDoesExits_Roundtrip)
      {
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

        // Off <- Off, self transition
        auto result = sm.dispatch<Triggers::OffToOff>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(2, OffState::DoitCalls);

        // Off <- Off, unhandled trigger
        result = sm.dispatch<Triggers::Off>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(0, OnState::EntryCalls);
        Assert::AreEqual<int>(0, OnState::DoitCalls);
        Assert::AreEqual<int>(0, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(2, OffState::DoitCalls);

        // On <- Off
        result = sm.dispatch<Triggers::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(1, OnState::DoitCalls);
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(2, OffState::DoitCalls);

        // On <- On, self transition
        result = sm.dispatch<Triggers::OnToOn>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(2, OnState::DoitCalls);
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(2, OffState::DoitCalls);

        // On <- On, unhandled trigger
        result = sm.dispatch<Triggers::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(0, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(2, OnState::DoitCalls);
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(1, OffState::EntryCalls);
        Assert::AreEqual<int>(2, OffState::DoitCalls);

        // Off <- On, unhandled trigger
        result = sm.dispatch<Triggers::Off>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        Assert::AreEqual<int>(1, OnState::ExitCalls);
        Assert::AreEqual<int>(1, OnState::EntryCalls);
        Assert::AreEqual<int>(2, OnState::DoitCalls);
        Assert::AreEqual<int>(1, OffState::ExitCalls);
        Assert::AreEqual<int>(2, OffState::EntryCalls);
        Assert::AreEqual<int>(3, OffState::DoitCalls);

        // Active state is Off
        Assert::AreEqual<int>(FactorCreatorFake<OffState>::CreateCalls, FactorCreatorFake<OffState>::DeleteCalls + 1);
        Assert::AreEqual<int>(FactorCreatorFake<OnState>::CreateCalls, FactorCreatorFake<OnState>::DeleteCalls);
      }
    };
  }
}
