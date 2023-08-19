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
#include "TestHelpers.h"

#define CAT(A, B) A##B
#define WSTRING(A) CAT(L, #A)

namespace UT {
  namespace Transitions {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace ToAndFromNestedStatesTestImpl {

      typedef State<MemoryAddressComparator, true> StateType;
      typedef SingletonCreatorFake<StateType> StateTypeCreationPolicyType;
      typedef Recorder<sizeof(__FILE__) + __LINE__> RecorderType;
      template<typename Derived> struct Leaf : BasicState<Derived, StateType>, SingletonCreatorFake<Derived> {};
      template<typename Derived, typename Statemachine> struct Composite : SubstatesHolderState<Derived, StateType, Statemachine>, SingletonCreatorFake<Derived> {};

      struct InitialStateFake : StateType {
        static const char* Name;
      };
      const char* InitialStateFake::Name = "Initial";

      enum Trigger {
        A_B,
        A_BA,
        B_A,
        BA_BB,
        BB_BA,
        BB_A,
        BBB_A,
      };

      struct A;
      struct B;
      struct BA;
      struct BB;
      struct BBA;
      struct BBB;

      struct A : Leaf<A> {
        static const char* Name;
        void entry() { RecorderType::add("A::Entry"); }
        void exit() { RecorderType::add("A::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("A::Do"); }
      };
      const char* A::Name = "A";

      typedef Transition<Trigger::BB_BA, BB, BA, StateTypeCreationPolicyType, OkGuard, ActionSpy<BB, BA, RecorderType>> BB_BA_t;
      typedef Transition<Trigger::BA_BB, BA, BB, StateTypeCreationPolicyType, OkGuard, ActionSpy<BA, BB, RecorderType>> BA_BB_t;
      typedef ExitTransition<Trigger::A_BA, A, BA, StateTypeCreationPolicyType, OkGuard, ActionSpy<A, BA, RecorderType>> A_BA_t;
      typedef EntryDeclaration<Trigger::BB_A, BB, StateTypeCreationPolicyType, ActionSpy<BB, B, RecorderType>> BB_B_t;
      typedef EntryDeclaration<Trigger::BBB_A, BB, StateTypeCreationPolicyType, ActionSpy<BBB, B, RecorderType>> BBB_B_t;
      typedef
        Typelist<BB_BA_t,
        Typelist<BA_BB_t,
        Typelist<BB_B_t,
        Typelist<BBB_B_t,
        Typelist<A_BA_t,
        NullType>>>>> B_transitions;
      typedef InitialTransition<BA, StateTypeCreationPolicyType, ActionSpy<BA, InitialStateFake, RecorderType>> B_initt;
      typedef Statemachine<
        B_transitions,
        B_initt> B_sm;

      struct B : Composite<B, B_sm> {
        static const char* Name;
        void entry() { RecorderType::add("B::Entry"); }
        void exit() { RecorderType::add("B::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("B::Do"); }
      };
      const char* B::Name = "B";

      struct BA : Leaf<BA> {
        static const char* Name;
        void entry() { RecorderType::add("BA::Entry"); }
        void exit() { RecorderType::add("BA::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("BA::Do"); }
      };
      const char* BA::Name = "BA";

      typedef EntryDeclaration<Trigger::BBB_A, BBB, StateTypeCreationPolicyType, ActionSpy<BBA, BB, RecorderType>> BBB_BB_t;
      typedef
        Typelist<BBB_BB_t,
        NullType> BB_transitions;
      typedef InitialTransition<BBA, StateTypeCreationPolicyType, ActionSpy<BBA, InitialStateFake, RecorderType>> BB_initt;
      typedef Statemachine<
        BB_transitions,
        BB_initt> BB_sm;

      struct BB : Composite<BB, BB_sm> {
        static const char* Name;
        void entry() { RecorderType::add("BB::Entry"); }
        void exit() { RecorderType::add("BB::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("BB::Do"); }
      };
      const char* BB::Name = "BB";

      struct BBA : Leaf<BBA> {
        static const char* Name;
        void entry() { RecorderType::add("BBA::Entry"); }
        void exit() { RecorderType::add("BBA::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("BBA::Do"); }
      };
      const char* BBA::Name = "BBA";

      struct BBB : Leaf<BBB> {
        static const char* Name;
        void entry() { RecorderType::add("BBB::Entry"); }
        void exit() { RecorderType::add("BBB::Exit"); }
        template<uint8_t N>
        void doit() { RecorderType::add("BBB::Do"); }
      };
      const char* BBB::Name = "BBB";

      typedef Transition<Trigger::B_A, B, A, StateTypeCreationPolicyType, OkGuard, ActionSpy<B, A, RecorderType>> B_A_t;
      typedef Transition<Trigger::A_B, A, B, StateTypeCreationPolicyType, OkGuard, ActionSpy<A, B, RecorderType>> A_B_t;
      typedef Transition<Trigger::BB_A, B, A, StateTypeCreationPolicyType, OkGuard, ActionSpy<BB, A, RecorderType>> BB_A_t;
      typedef Transition<Trigger::BBB_A, B, A, StateTypeCreationPolicyType, OkGuard, ActionSpy<BBB, A, RecorderType>> BBB_A_d;
      typedef Declaration<Trigger::BB_BA, B, StateTypeCreationPolicyType> BB_BA_d;
      typedef Declaration<Trigger::BA_BB, B, StateTypeCreationPolicyType> BA_BB_d;
      typedef ExitDeclaration<Trigger::A_BA, A, B, StateTypeCreationPolicyType> A_BA_d;
      typedef
        Typelist<B_A_t,
        Typelist<A_B_t,
        Typelist<BB_A_t,
        Typelist<BBB_A_d,
        Typelist<BB_BA_d,
        Typelist<BA_BB_d,
        Typelist<A_BA_d,
        NullType>>>>>>> Toplevel_transitions;
    }

    TEST_CLASS(ToAndFromNestedStates)
    {
    public:
      TEST_METHOD_INITIALIZE(Initialize)
      {
        using namespace ToAndFromNestedStatesTestImpl;
        SingletonCreatorFake<A>::reset();
        SingletonCreatorFake<B>::reset();
        SingletonCreatorFake<BA>::reset();
        SingletonCreatorFake<BB>::reset();
        SingletonCreatorFake<BBA>::reset();
        SingletonCreatorFake<BBB>::reset();
        StateTypeCreationPolicyType::reset();
      }

      TEST_METHOD(Callsequence_B__A)
      {
        using namespace ToAndFromNestedStatesTestImpl;
        typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake, RecorderType>> ToplevelInitTransition;
        typedef Statemachine<
          Toplevel_transitions,
          ToplevelInitTransition> Toplevel_sm;

        Toplevel_sm sm;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::B_A>();
        RecorderType::check({
          "B<-A",
          "A::Exit",
          "B::Entry",
          "BA<-Initial",
          "BA::Entry",
          "BA::Do" });

        // Active state is B/BA
        Assert::AreNotEqual<int>(0, SingletonCreatorFake<A>::createCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, SingletonCreatorFake<BA>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
      }

      TEST_METHOD(Callsequence_A__B)
      {
        using namespace ToAndFromNestedStatesTestImpl;
        typedef InitialTransition<B, StateTypeCreationPolicyType, ActionSpy<B, InitialStateFake, RecorderType>> ToplevelInitTransition;
        typedef Statemachine<
          Toplevel_transitions,
          ToplevelInitTransition> Toplevel_sm;

        Toplevel_sm sm;

        sm.begin();
        RecorderType::check({
          "B<-Initial",
          "B::Entry",
          "BA<-Initial",
          "BA::Entry",
          "BA::Do" });

        sm.dispatch<Trigger::A_B>();
        RecorderType::check({
          "A<-B",
          "BA::Exit",
          "B::Exit",
          "A::Entry",
          "A::Do" });

        // Active state is A
        Assert::AreNotEqual<int>(0, SingletonCreatorFake<B>::createCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
        // Finalize is called and the sub-state is exited using polymorphism. StateTypeCreationPolicyType is used to delete the state.
        Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, StateTypeCreationPolicyType::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
      }

      TEST_METHOD(Callsequence_BB__A)
      {
        using namespace ToAndFromNestedStatesTestImpl;
        typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake, RecorderType>> ToplevelInitTransition;
        typedef Statemachine<
          Toplevel_transitions,
          ToplevelInitTransition> Toplevel_sm;

        Toplevel_sm sm;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::BB_A>();
        RecorderType::check({
          "BB<-A",
          "A::Exit",
          "B::Entry",
          "BB::Entry",
          "BBA<-Initial",
          "BBA::Entry",
          "BBA::Do" });

        // Active state is B/BB/BBA
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, SingletonCreatorFake<BA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
      }

      TEST_METHOD(Callsequence_BBB__A)
      {
        using namespace ToAndFromNestedStatesTestImpl;
        typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake, RecorderType>> ToplevelInitTransition;
        typedef Statemachine<
          Toplevel_transitions,
          ToplevelInitTransition> Toplevel_sm;

        Toplevel_sm sm;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::BBB_A>();
        RecorderType::check({
          "BBB<-A",
          "A::Exit",
          "B::Entry",
          "BB::Entry",
          "BBB::Entry",
          "BBB::Do" });
      }

      TEST_METHOD(Callsequence_BB__BA)
      {
        using namespace ToAndFromNestedStatesTestImpl;
        typedef InitialTransition<B, StateTypeCreationPolicyType, ActionSpy<B, InitialStateFake, RecorderType>> ToplevelInitTransition;
        typedef Statemachine<
          Toplevel_transitions,
          ToplevelInitTransition> Toplevel_sm;

        Toplevel_sm sm;

        sm.begin();
        RecorderType::check({
          "B<-Initial",
          "B::Entry",
          "BA<-Initial",
          "BA::Entry",
          "BA::Do" });

        sm.dispatch<Trigger::BB_BA>();
        RecorderType::check({
          "BB<-BA",
          "BA::Exit",
          "BB::Entry",
          "BBA<-Initial",
          "BBA::Entry",
          "BBA::Do" });

        sm.dispatch<Trigger::BA_BB>();
        RecorderType::check({
          "BA<-BB",
          "BBA::Exit",
          "BB::Exit",
          "BA::Entry",
          "BA::Do" });
      }

      TEST_METHOD(Callsequence_A__BA)
      {
        using namespace ToAndFromNestedStatesTestImpl;
        typedef InitialTransition<B, StateTypeCreationPolicyType, ActionSpy<B, InitialStateFake, RecorderType>> ToplevelInitTransition;
        typedef Statemachine<
          Toplevel_transitions,
          ToplevelInitTransition> Toplevel_sm;

        Toplevel_sm sm;

        sm.begin();
        RecorderType::check({
          "B<-Initial",
          "B::Entry",
          "BA<-Initial",
          "BA::Entry",
          "BA::Do" });

        sm.dispatch<Trigger::A_BA>();
        RecorderType::check({
          "A<-BA",
          "BA::Exit",
          "B::Exit",
          "A::Entry",
          "A::Do" });
      }
    };
  }
}
