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
    using namespace LokiLight;
    using namespace UnitTests::Helpers;

    namespace StatemachineOnOffCallSequenceTestImpl {

      using StatePolicy = State<VirtualTypeIdComparator, false>;
      using RecorderType = Recorder<sizeof(__FILE__) + __LINE__>;

      struct InitialStateFake : StatePolicy {
        using Policy = StatePolicy;
        static const char* name;
      };
      const char* InitialStateFake::name = "Initial";

      struct FinalStateFake : BasicState<FinalStateFake, StatePolicy, true, true, true> {
        static const char* name;
        using CreatorType = FinalStateFake;
        using ObjectType = FinalStateFake;
        static FinalStateFake* create() { return nullptr; }
        static void destroy(FinalStateFake*) { }

      private:
        friend class BasicState<FinalStateFake, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event&) { }
        template<class Event> void exit(const Event&) { }
        template<class Event> void doit(const Event&) { }
      };
      const char* FinalStateFake::name = "Final";

      namespace Trigger {
        struct On {};
        struct Off {};
        struct OnToOn {};
        struct OffToOff {};
        struct OffToFinal {};
      }

      struct OnState : BasicState<OnState, StatePolicy, true, true, true>, FactoryCreator<OnState> {
        static const char* name;
        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<OnState, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event&) { RecorderType::add("OnState::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("OnState::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("OnState::Do"); }
      };
      const char* OnState::name = "OnState";

      struct OffState : BasicState<OffState, StatePolicy, true, true, true>, FactoryCreator<OffState> {
        static const char* name;
        uint8_t getTypeId() const override { return 2; }

      private:
        friend class BasicState<OffState, StatePolicy, true, true, true>;
        template<class Event>  void entry(const Event&) { RecorderType::add("OffState::Entry"); }
        template<class Event>  void exit(const Event&) { RecorderType::add("OffState::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("OffState::Do"); }
      };
      const char* OffState::name = "OffState";

      using ToOnFromOffActionSpy = ActionSpy<struct OnState, struct OffState, RecorderType>;
      using ToOffFromOnActionSpy = ActionSpy<struct OffState, struct OnState, RecorderType>;
      using ToOnFromOnActionSpy = ActionSpy<struct OnState, struct OnState, RecorderType>;
      using ToOffFromOffActionSpy = ActionSpy<struct OffState, struct OffState, RecorderType>;
      using ToFinalFromOffActionSpy = ActionSpy<struct FinalStateFake, struct OffState, RecorderType>;

      using ToOnFromOffTransition = Transition<Trigger::On, OnState, OffState, NoGuard, ToOnFromOffActionSpy>;
      using ToOffFromOnTransition = Transition<Trigger::Off, OffState, OnState, NoGuard, ToOffFromOnActionSpy>;
      using ToOnFromOnTransition = SelfTransition<Trigger::OnToOn, OnState, NoGuard, ToOnFromOnActionSpy, false>;
      using ToOffFromOffTransition = SelfTransition<Trigger::OffToOff, OffState, NoGuard, ToOffFromOffActionSpy, false>;
      using ToFinalFromOffTransition = Transition<Trigger::OffToFinal, FinalStateFake, OffState, NoGuard, ToFinalFromOffActionSpy>;

      using TransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToOnFromOnTransition,
        Typelist<ToOffFromOffTransition,
        Typelist<ToFinalFromOffTransition,
        NullType>>>>>;

      using ToOffFromInitialActionSpy = ActionSpy<struct OffState, struct InitialStateFake, RecorderType>;
      using InitTransition = InitialTransition<OffState, ToOffFromInitialActionSpy>;
      using Sm = Statemachine<TransitionList, InitTransition>;
    }

    BEGIN(StatemachineOnOffCallSequenceTest)
    
      INIT(
        Initialize,
        {
          using namespace StatemachineOnOffCallSequenceTestImpl;
          RecorderType::reset();
        })

      TEST(
        Roundtrip,
        EntriesDoesExits,
        Successful)
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
        EQ(off.getTypeId(), result.activeState->getTypeId());
        RecorderType::check({
          "OffState<-OffState",
          "OffState::Do" });

        // Off <- Off, unhandled trigger
        result = sm.dispatch<Trigger::Off>();

        // On <- Off
        result = sm.dispatch<Trigger::On>();
        EQ(on.getTypeId(), result.activeState->getTypeId());
        RecorderType::check({
          "OnState<-OffState",
          "OffState::Exit",
          "OnState::Entry",
          "OnState::Do" });

        // On <- On, self transition
        result = sm.dispatch<Trigger::OnToOn>();
        EQ(on.getTypeId(), result.activeState->getTypeId());
        RecorderType::check({
         "OnState<-OnState",
         "OnState::Do" });

        // On <- On, unhandled trigger
        result = sm.dispatch<Trigger::On>();
        EQ(on.getTypeId(), result.activeState->getTypeId());

        // Off <- On, unhandled trigger
        result = sm.dispatch<Trigger::Off>();
        EQ(off.getTypeId(), result.activeState->getTypeId());
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

    END

  }
}
