/*
  Copyright 2023 Stefan Grimm

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
  namespace Transitions {

    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace ChoiceTransitionSubstateTestImplImpl {

      using StatePolicy = State<VirtualTypeIdComparator, false>;
      using RecorderType = Recorder<sizeof(__FILE__) + __LINE__>;
      template<class Derived> struct Leaf : BasicState<Derived, StatePolicy, true, true, true>, FactoryCreatorFake<Derived> {};
      template<class Derived, class Statemachine> struct Composite : SubstatesHolderState<Derived, StatePolicy, Statemachine, true, true>, FactoryCreatorFake<Derived> {};

      template<class From>
      struct ActionChoiceRecordingSpy {
        template<class StateType, class EventType>
        void perform(StateType&, const EventType&) {
          ostringstream buf;
          buf << "?<-" << From::name;
          RecorderType::add(buf.str());
        }
      };

      namespace Trigger
      {
        struct Choice_A_B {};
        struct Choice_A_B_C {};
      }

      struct B : Leaf<B> {
        static const char* name;

        template<class Event> void entry(const Event&) { RecorderType::add("B::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("B::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("B::Do"); }
        uint8_t getTypeId() const override { return 2; };
      };
      const char* B::name = "B";

      struct C : Leaf<C> {
        static const char* name;

        template<class Event> void entry(const Event&) { RecorderType::add("C::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("C::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("C::Do"); }
        uint8_t getTypeId() const override { return 3; };
      };
      const char* C::name = "C";

      struct AA : Leaf<AA> {
        static const char* name;
        
        template<class Event> void entry(const Event&) { RecorderType::add("AA::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("AA::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("AA::Do"); }

        uint8_t getTypeId() const override { return 11; };
      };
      const char* AA::name = "AA";

      struct ChoiceGuardStub {
        static bool ReturnValue;
        template<class StateType, class EventType>
        bool eval(const StateType&, const EventType&) {
          return ReturnValue;
        }
      };
      bool ChoiceGuardStub::ReturnValue = false;

      struct Choice1GuardStub {
        static bool ReturnValue;
        template<class StateType, class EventType>
        bool eval(const StateType&, const EventType&) {
          return ReturnValue;
        }
      };
      bool Choice1GuardStub::ReturnValue = false;

      struct Choice2GuardStub {
        static bool ReturnValue;
        template<class StateType, class EventType>
        bool eval(const StateType&, const EventType&) {
          return ReturnValue;
        }
      };
      bool Choice2GuardStub::ReturnValue = false;

      using ToBorSelfAA = ChoiceExitTransition<Trigger::Choice_A_B, B, AA, AA, ChoiceGuardStub, ActionChoiceRecordingSpy<AA>>;
      using ToBorCorSelfAA = Choice2ExitTransition<Trigger::Choice_A_B_C, B, C, AA, AA, Choice1GuardStub, Choice2GuardStub, ActionChoiceRecordingSpy<AA>>;
      using ToFinalFromAA = FinalTransition<AA>;
      using ATransitions =
        Typelist<ToBorSelfAA,
        Typelist<ToBorCorSelfAA,
        Typelist<ToFinalFromAA,
        NullType>>>;

      using AInitTransition = InitialTransition<AA, ActionSpy<AA, InitialStateNamedFake<StatePolicy>, RecorderType>>;
      using ASm = Statemachine<ATransitions, AInitTransition>;

      struct A : Composite<A, ASm> {
        static const char* name;

        template<class Event> void entry(const Event&) { RecorderType::add("A::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("A::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("A::Do"); }

        uint8_t getTypeId() const override { return 1; };
      };
      const char* A::name = "A";

      using DeclToBorSelfAA = ExitDeclaration<Trigger::Choice_A_B, B, A>;
      using DeclToBorCorSelfAA = Exit2Declaration<Trigger::Choice_A_B_C, B, C, A, Choice1GuardStub>;
      using Transitions =
        Typelist<DeclToBorSelfAA,
        Typelist<DeclToBorCorSelfAA,
        NullType>>;

      using ToplevelInitTransition = InitialTransition<A, ActionSpy<A, InitialStateNamedFake<StatePolicy>, RecorderType>>;
      using Sm = Statemachine<Transitions, ToplevelInitTransition>;
    }

    BEGIN(
      ChoiceTransitionSubstateTestImpl,
      "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=ChoiceTransitionSubstateTest.drawio#R7Vtbc5s6EP41njl9cIa77cfYiXtJ05M5zrTNeekQkIFWIEfIscmvP1IQNkgCEwd82pm4My5aXYDdb1ffrpWBOYu377G7Cq%2BRD%2BDA0PztwLwYGMbEceg3E2S5wJnYuSDAkZ%2BL9L1gET0BLtS4dB35IK0MJAhBEq2qQg8lCfBIReZijDbVYUsEq3dduQGQBAvPhbL0W%2BSTMJeObW0v%2FwCiICzurGu8J3aLwVyQhq6PNiWReTkwZxghkl%2FF2xmATHeFXvJ585re3YNhkJA2E8jm5hbAjFyef9xsk8unm8fL8ZCv8ujCNX%2FhNOaPS7JCB%2FTJV%2BxyHcM5dmN6Od2EEQGLlesx%2BYZanspCEkPa0uklRuvEBz5v7d5cow0PxZHHr6F7D%2BDU9X4FzxNmCCJMuxKUsHukBKNfO62zhZYoIXM3jiAD01eAfTdxuZgjR6dqnbowChLa8KhmAF1wKquqeG%2BACdiWRFx17wGKAcEZHcJ7TW7FrNrc7DExKTARlvBgF0KX4zDYrbw3Fb3g1lJb7sPPbDu%2FWa8ehjfAud6st4s7qLDcVDJc1QwHbOZijyvRsJhSIwgLgwwMc0k%2FnrazSqVHo5%2FjzdzSqKxduqum8btKxmY2jagDn%2FMOglbdQMCxqhgY2xIGDEuBAb0Iea%2FBgNJ7DQkD528Y6BUDhogBOQ6YugID1qSDOKDEgClj4A0EPYPAqYJAt2UUGCMFCswudgMlCsaSyYFPeQxvIkxCFKDEhZd7aWmTZurbj%2FmMmKKeofATEJJx5btrgqpAqVVlitbYAw2PO%2BFMzsUBIIcDHHuXRsNgAF0SPVY5W%2BdKnkiuJmsdQspMQVVPBX9K6fuSBXEJkN1qB%2BN6t%2BqEVHXtZH6EKe2OEBNSszP4d%2BFiuuBiliLOqhysL%2F%2FSLcn2sxBFHvhx%2FmP6Y9YQcI%2F0Lao6nH0vN%2B7YYmd20bzY8sXzVla0thH5XrrOZ41s3tzPYo1i0vGObMqOPIafra9WBj3LCz%2BuHjL7C7pv7cjc%2FEPtTKefapTltm3t63z1GxTRt9oPQctlSp9VRMTuIVqBpOklm%2BIDDlF8v04Pb8Id%2BJA5Fn3odE6k1I8t68eeBmsX%2B1Sqs9kpwxKmvfZFDy5VuMaZXXaORs%2FYuaFWdcPRQT88uP22cromnJWdrilFfIHTOY5mVTAz1O3Xed1W4L%2BFM4t8OH9pPqtcxRAW2iXUdQvlWmlYqD4OnGPsZqURKzYjrX8ps0jsiqKApTU%2F%2B0h71XiaWgg%2BmD9xp0FMsdNJrngogUhXeSluGW2Zw3YSzAQAWSM5mKkSL72DYBaBETbuHu4Wi29T%2B%2BnDlRdfXSjqL%2FfscRcELN2EXrzHURwPDLq49tfsHf2m%2F8y8DSlvSFLAoh4LaZj5cgjo94z1Tu%2BGC5rBadaZxuZGjGglzM%2BZMCSEFT7P2bMbcw%2BDZ6ek9C9GSXqGcEClfHkK3Pl9Nkxd%2Bv%2FzWvN3kiGp8onAVSv0syCNJa7KRQURhGBJFLlWHPk%2BrCsUViN5iXpOeqnV2Yok3VZtfEZfWFERAweSZ%2Fr8SC8DdnmLoyAAOLdu3ktvVhqgmLPnoEdMYbQ1777H4gRxmVPh5h4RguJWuOkAKEIxx3JUMUUVVLqIKl%2B8KczCyd3tv9YnhLzlU%2FCoqurKPOiUFNKu7nDUcSQN9UUhlfqRPansAr0lYQL7Oy4JM06YhDWB6wV8ULNGTpVuvTIJKypoIotzhBL5QTpYz%2BJkmiaAWLpZDWU8glEp1a4gVPa0Lss5QdmuDtVGW1jrFVh3gOomtPaQ5dhja1JFRDdJjikWrcRtom2SIybu4%2B5ynK5Arczlly5M1ahO%2FHP2Y%2FyeByh%2FLK5m3Pswe1dG7qGY%2B4Ls%2FnhwOr2FXKnutUvBy%2FRV9VNzN6FZzNSN0UtDc03GZmrtwm59Fv5%2F4NyRcf47ZMOOwMsmp8uGl%2FNP1uLJufh5vd3GT2Gwhf9cqfe4PsOBdlQ40F8fDtpW5BqLK39QPBiJ8UDcjNrGA2kh4zePB0q2Ibv%2FSRlbF07QwZ6oVI0iDWlKV7r7UbemejqxzgTirx%2B5lY0caSmrHSXrCnbyKQ8Zh28%2FPfdzukOX99a%2Bah5K28t76%2BPLjE%2B32%2BNN3wVbESK%2FrTg9eVKNKpKXPz6qd0Bt2haXmlB6MKrvWExH9EQ8mDsSlmgd40fij3nCQr8bPZk%2BpKu%2Fh1%2FXYRaOHq%2Buk9D%2B9E1VH5Wg%2FXYYcFA9DChFMoWn1AY38VSwqThl0d%2BxYCUIFGRhf86C9ZzmnMVLyoRHV9pbHrMQwlJnWV69EzbldJamjSugGepGJ9HQ0qpB7OgKpCMU6I2W0bCPYxa2WA7lNL722IQw3pyMBZd60bEJ2tz%2FkU4%2BfP%2BXTublfw%3D%3D")
   
      INIT(
        Initialize,
        {
          using namespace ChoiceTransitionSubstateTestImplImpl;
          RecorderType::reset();
          FactoryCreatorFake<A>::reset();
          FactoryCreatorFake<AA>::reset();
          FactoryCreatorFake<B>::reset();
          FactoryCreatorFake<C>::reset();
          ChoiceGuardStub::ReturnValue = false;
          Choice1GuardStub::ReturnValue = false;
          Choice2GuardStub::ReturnValue = false;
        })

      TEST(
        Choice_A_B,
        WhenChoiceReturnsTrue,
        ActiveState_B)
      {
        using namespace ChoiceTransitionSubstateTestImplImpl;
        Sm sm;
        ChoiceGuardStub::ReturnValue = true;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "AA<-Initial",
          "AA::Entry",
          "AA::Do" });

        sm.dispatch<Trigger::Choice_A_B>();
        RecorderType::check({
          "?<-AA",
          "AA::Exit",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        NEQ(0, FactoryCreatorFake<B>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<AA>::createCalls, FactoryCreatorFake<AA>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST(
        Choice_A_B,
        WhenChoiceReturnsFalse,
        ActiveState_A_AA)
      {
        using namespace ChoiceTransitionSubstateTestImplImpl;
        Sm sm;
        ChoiceGuardStub::ReturnValue = false;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "AA<-Initial",
          "AA::Entry",
          "AA::Do" });

        sm.dispatch<Trigger::Choice_A_B>();
        RecorderType::check({
          "?<-AA",
          "AA::Do" });

        // Active state is B
        NEQ(0, FactoryCreatorFake<A>::createCalls);
        NEQ(0, FactoryCreatorFake<AA>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls + 1);
        EQ(FactoryCreatorFake<AA>::createCalls, FactoryCreatorFake<AA>::deleteCalls + 1);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST(
        Choice_A_B_C,
        WhenChoice1ReturnsTrue,
        ActiveState_B)
      {
        using namespace ChoiceTransitionSubstateTestImplImpl;
        Sm sm;
        Choice1GuardStub::ReturnValue = true;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "AA<-Initial",
          "AA::Entry",
          "AA::Do" });

        sm.dispatch<Trigger::Choice_A_B_C>();
        RecorderType::check({
          "?<-AA",
          "AA::Exit",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        NEQ(0, FactoryCreatorFake<B>::createCalls);
        EQ(FactoryCreatorFake<AA>::createCalls, FactoryCreatorFake<AA>::deleteCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST(
        Choice_A_B_C,
        WhenChoice2ReturnsTrue,
        ActiveState_C)
      {
        using namespace ChoiceTransitionSubstateTestImplImpl;
        Sm sm;
        Choice2GuardStub::ReturnValue = true;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "AA<-Initial",
          "AA::Entry",
          "AA::Do" });

        sm.dispatch<Trigger::Choice_A_B_C>();
        RecorderType::check({
          "?<-AA",
          "AA::Exit",
          "A::Exit",
          "C::Entry",
          "C::Do" });

        // Active state is C
        NEQ(0, FactoryCreatorFake<C>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<AA>::createCalls, FactoryCreatorFake<AA>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls + 1);
      }

      TEST(
        Choice_A_B_C,
        WhenChoice1andChoice2ReturnFalse,
        ActiveState_A)
      {
        using namespace ChoiceTransitionSubstateTestImplImpl;
        Sm sm;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "AA<-Initial",
          "AA::Entry",
          "AA::Do" });

        sm.dispatch<Trigger::Choice_A_B_C>();
        RecorderType::check({
          "?<-AA",
          "AA::Do" });

        // Active state is A
        NEQ(0, FactoryCreatorFake<A>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls + 1);
        EQ(FactoryCreatorFake<AA>::createCalls, FactoryCreatorFake<AA>::deleteCalls + 1);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

    END

  }
}
