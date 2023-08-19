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

#include "..\..\src\state.h"
#include "..\..\src\lokilight.h"
#include "..\..\src\statemachine.h"
#include "..\..\src\transition.h"
#include "..\..\src\initialtransition.h"
#include "..\..\src\finaltransition.h"
#include "TestHelpers.h"

namespace UT {
  namespace StatemachineOnOff {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace StatemachineOnOffCallSequenceTestImpl {

      typedef State<VirtualGetTypeIdStateComparator, false> StateType;
      typedef FactoryCreator<StateType, false> StateTypeCreationPolicyType;
      typedef Recorder<sizeof(__FILE__) + __LINE__> RecorderType;

      struct InitialStateFake : StateType {
        static const char* name;
      };
      const char* InitialStateFake::name = "Initial";

      struct FinalStateFake : BasicState<FinalStateFake, StateType> {
        static const char* name;
        typedef FinalStateFake CreatorType;
        typedef FinalStateFake ObjectType;
        static FinalStateFake* create() { return nullptr; }
        static void destroy(FinalStateFake*) { }

      private:
        friend class BasicState<FinalStateFake, StateType>;
        void entry() { }
        void exit() { }
        template<uint8_t N>
        void doit() { }
      };
      const char* FinalStateFake::name = "Final";

      enum Trigger {
        On,
        Off,
        OnToOn,
        OffToOff,
        OffToFinal
      };

      struct OnState : BasicState<OnState, StateType>, FactoryCreator<OnState> {
        static const char* name;
        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<OnState, StateType>;
        void entry() { RecorderType::add("OnState::Entry"); }
        void exit() { RecorderType::add("OnState::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("OnState::Do"); }
      };
      const char* OnState::name = "OnState";

      struct OffState : BasicState<OffState, StateType>, FactoryCreator<OffState> {
        static const char* name;
        uint8_t getTypeId() const override { return 2; }

      private:
        friend class BasicState<OffState, StateType>;
        void entry() { RecorderType::add("OffState::Entry"); }
        void exit() { RecorderType::add("OffState::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("OffState::Do"); }
      };
      const char* OffState::name = "OffState";

      typedef ActionSpy<struct OnState, struct OffState, RecorderType> ToOnFromOffActionSpy;
      typedef ActionSpy<struct OffState, struct OnState, RecorderType> ToOffFromOnActionSpy;
      typedef ActionSpy<struct OnState, struct OnState, RecorderType> ToOnFromOnActionSpy;
      typedef ActionSpy<struct OffState, struct OffState, RecorderType> ToOffFromOffActionSpy;
      typedef ActionSpy<struct FinalStateFake, struct OffState, RecorderType> ToFinalFromOffActionSpy;

      typedef Transition<Trigger::On, OnState, OffState, StateTypeCreationPolicyType, NoGuard, ToOnFromOffActionSpy> ToOnFromOffTransition;
      typedef Transition<Trigger::Off, OffState, OnState, StateTypeCreationPolicyType, NoGuard, ToOffFromOnActionSpy> ToOffFromOnTransition;
      typedef SelfTransition<Trigger::OnToOn, OnState, StateTypeCreationPolicyType, NoGuard, ToOnFromOnActionSpy, false> ToOnFromOnTransition;
      typedef SelfTransition<Trigger::OffToOff, OffState, StateTypeCreationPolicyType, NoGuard, ToOffFromOffActionSpy, false> ToOffFromOffTransition;
      typedef Transition<Trigger::OffToFinal, FinalStateFake, OffState, StateTypeCreationPolicyType, NoGuard, ToFinalFromOffActionSpy> ToFinalFromOffTransition;

      typedef
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToOnFromOnTransition,
        Typelist<ToOffFromOffTransition,
        Typelist<ToFinalFromOffTransition,
        NullType>>>>> TransitionList;

      typedef ActionSpy<struct OffState, struct InitialStateFake, RecorderType> ToOffFromInitialActionSpy;
      typedef InitialTransition<OffState, StateTypeCreationPolicyType, ToOffFromInitialActionSpy> InitTransition;
      typedef Statemachine<
        TransitionList,
        InitTransition> Sm;
    }

    TEST_CLASS(StatemachineOnOffCallSequenceTest)
    {
    public:
      TEST_METHOD_INITIALIZE(Initialize)
      {
        using namespace StatemachineOnOffCallSequenceTestImpl;
        RecorderType::reset();
      }

      TEST_METHOD(EntriesDoesExits_Roundtrip)
      {
        using namespace StatemachineOnOffCallSequenceTestImpl;
        RecorderType::reset();

        OnState on;
        OffState off;

        Sm sm;
        sm.begin();
        RecorderType::check({
          "OffState<-Initial",
          "OffState::Entry",
          "OffState::Do" });

        // Off <- Off, self transition
        auto result = sm.dispatch<Trigger::OffToOff>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        RecorderType::check({
          "OffState<-OffState",
          "OffState::Do" });

        // Off <- Off, unhandled trigger
        result = sm.dispatch<Trigger::Off>();

        // On <- Off
        result = sm.dispatch<Trigger::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        RecorderType::check({
          "OnState<-OffState",
          "OffState::Exit",
          "OnState::Entry",
          "OnState::Do" });

        // On <- On, self transition
        result = sm.dispatch<Trigger::OnToOn>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        RecorderType::check({
         "OnState<-OnState",
         "OnState::Do" });

        // On <- On, unhandled trigger
        result = sm.dispatch<Trigger::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());

        // Off <- On, unhandled trigger
        result = sm.dispatch<Trigger::Off>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        RecorderType::check({
         "OffState<-OnState",
         "OnState::Exit",
         "OffState::Entry",
         "OffState::Do" });

        result = sm.dispatch<Trigger::OffToFinal>();
        RecorderType::check({
         "Final<-OffState",
         "OffState::Exit" });
      }
    };
  }
}
