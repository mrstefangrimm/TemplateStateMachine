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
#define IAMWORKSTATION 1

#include "CppUnitTest.h"

#include "..\..\src\state.h"
#include "..\..\src\templatemeta.h"
#include "..\..\src\statemachine.h"
#include "..\..\src\transition.h"
#include "TestHelpers.h"

#include <vector>

#define CAT(A, B) A##B
#define WSTRING(A) CAT(L, #A)

// Unit tests
namespace UT {
  // State machine
  namespace SM {
    // Call sequence
    namespace CS {

      using namespace Microsoft::VisualStudio::CppUnitTestFramework;
      using namespace tsmlib;
      using namespace std;
      using namespace UnitTests::Helpers;

      typedef State<MemoryAddressStateComperator<true>, true> StateType;
      typedef SingletonCreatorFake<StateType> StateTypeCreationPolicyType;
      template<typename Derived> struct Leaf : SimpleState<Derived, StateType>, SingletonCreatorFake<Derived> {};
      template<typename Derived, typename Statemachine> struct Composite : SubstatesHolderState<Derived, StateType, Statemachine>, SingletonCreatorFake<Derived> {};

      struct InitialStateFake : StateType {
        static const char* Name;
      };
      const char* InitialStateFake::Name = "Initial";

      template<typename TO, typename FROM>
      struct ActionSpy {
        template<typename T>
        void perform(T*) {
          ostringstream buf;
          buf << TO::Name << "<-" << FROM::Name;
          recorder.push_back(buf.str());
        }
      };

      enum Triggers {
        A_B,
        A_BA,
        B_A,
        BA_BB,
        BB_BA,
        BB_A,
        BBB_A,
      };

      vector<string> recorder;

      struct A;
      struct B;
      struct BA;
      struct BB;
      struct BBA;
      struct BBB;

      typedef ActionSpy<B, A> B_A_spy;
      typedef ActionSpy<A, B> A_B_spy;
      typedef ActionSpy<BB, BA> BB_BA_spy;
      typedef ActionSpy<BA, BB> BA_BB_spy;
      typedef ActionSpy<BB, A> BB_A_spy;
      typedef ActionSpy<BBB, A> BBB_A_spy;
      typedef ActionSpy<BB, B> BB_B_spy;
      typedef ActionSpy<BBB, B> BBB_B_spy;
      typedef ActionSpy<BBA, BB> BBB_BB_spy;
      typedef ActionSpy<BA, InitialStateFake> B_Init_spy;
      typedef ActionSpy<BBA, InitialStateFake> BB_Init_spy;

      struct A : Leaf<A> {
        static const char* Name;
        void entry() { recorder.push_back("A::Entry"); }
        void exit() { recorder.push_back("A::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("A::Do"); }
      };
      const char* A::Name = "A";

      typedef Transition<Triggers::BB_BA, BB, BA, StateTypeCreationPolicyType, OkGuard, BB_BA_spy> BB_BA_t;
      typedef Transition<Triggers::BA_BB, BA, BB, StateTypeCreationPolicyType, OkGuard, BA_BB_spy> BA_BB_t;
      typedef ExitTransition<Triggers::A_BA, A, BA, StateTypeCreationPolicyType, OkGuard, ActionSpy<A, BA>> A_BA_t;
      typedef EntryDeclaration<Triggers::BB_A, BB, StateTypeCreationPolicyType, BB_B_spy> BB_B_t;
      typedef EntryDeclaration<Triggers::BBB_A, BB, StateTypeCreationPolicyType, BBB_B_spy> BBB_B_t;
      typedef
        Typelist<BB_BA_t,
        Typelist<BA_BB_t,
        Typelist<BB_B_t,
        Typelist<BBB_B_t,
        Typelist<A_BA_t,
        NullType>>>>> B_transitions;
      typedef InitialTransition<BA, StateTypeCreationPolicyType, B_Init_spy> B_initt;
      typedef Statemachine<
        B_transitions,
        B_initt,
        NullEndTransition<StateTypeCreationPolicyType>> B_sm;

      struct B : Composite<B, B_sm> {
        static const char* Name;
        void entry() { recorder.push_back("B::Entry"); }
        void exit() { recorder.push_back("B::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("B::Do"); }
      };
      const char* B::Name = "B";

      struct BA : Leaf<BA> {
        static const char* Name;
        void entry() { recorder.push_back("BA::Entry"); }
        void exit() { recorder.push_back("BA::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("BA::Do"); }
      };
      const char* BA::Name = "BA";

      typedef EntryDeclaration<Triggers::BBB_A, BBB, StateTypeCreationPolicyType, BBB_BB_spy> BBB_BB_t;
      typedef
        Typelist<BBB_BB_t,
        NullType> BB_transitions;
      typedef InitialTransition<BBA, StateTypeCreationPolicyType, BB_Init_spy> BB_initt;
      typedef Statemachine<
        BB_transitions,
        BB_initt,
        NullEndTransition<StateTypeCreationPolicyType>> BB_sm;

      struct BB : Composite<BB, BB_sm> {
        static const char* Name;
        void entry() { recorder.push_back("BB::Entry"); }
        void exit() { recorder.push_back("BB::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("BB::Do"); }
      };
      const char* BB::Name = "BB";

      struct BBA : Leaf<BBA> {
        static const char* Name;
        void entry() { recorder.push_back("BBA::Entry"); }
        void exit() { recorder.push_back("BBA::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("BBA::Do"); }
      };
      const char* BBA::Name = "BBA";

      struct BBB : Leaf<BBB> {
        static const char* Name;
        void entry() { recorder.push_back("BBB::Entry"); }
        void exit() { recorder.push_back("BBB::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("BBB::Do"); }
      };
      const char* BBB::Name = "BBB";

      typedef Transition<Triggers::B_A, B, A, StateTypeCreationPolicyType, OkGuard, B_A_spy> B_A_t;
      typedef Transition<Triggers::A_B, A, B, StateTypeCreationPolicyType, OkGuard, A_B_spy> A_B_t;
      typedef Transition<Triggers::BB_A, B, A, StateTypeCreationPolicyType, OkGuard, BB_A_spy> BB_A_t;
      typedef Transition<Triggers::BBB_A, B, A, StateTypeCreationPolicyType, OkGuard, BBB_A_spy> BBB_A_d;
      typedef Declaration<Triggers::BB_BA, B, StateTypeCreationPolicyType> BB_BA_d;
      typedef Declaration<Triggers::BA_BB, B, StateTypeCreationPolicyType> BA_BB_d;
      typedef ExitDeclaration<Triggers::A_BA, A, B, StateTypeCreationPolicyType, OkGuard> A_BA_d;
      typedef
        Typelist<B_A_t,
        Typelist<A_B_t,
        Typelist<BB_A_t,
        Typelist<BBB_A_d,
        Typelist<BB_BA_d,
        Typelist<BA_BB_d,
        Typelist<A_BA_d,
        NullType>>>>>>> Toplevel_transitions;

      TEST_CLASS(ToAndFromNestedStates)
      {
      public:

        TEST_METHOD_INITIALIZE(Initialize)
        {
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
          typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake>> ToplevelInitTransition;
          typedef Statemachine<
            Toplevel_transitions,
            ToplevelInitTransition,
            NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::B_A>();
          expected.push_back("B<-A");
          expected.push_back("A::Exit");
          expected.push_back("B::Entry");
          expected.push_back("BA<-Initial");
          expected.push_back("BA::Entry");
          expected.push_back("BA::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

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
          typedef InitialTransition<B, StateTypeCreationPolicyType, ActionSpy<B, InitialStateFake>> ToplevelInitTransition;
          typedef Statemachine<
            Toplevel_transitions,
            ToplevelInitTransition,
            NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("B<-Initial");
          expected.push_back("B::Entry");
          expected.push_back("BA<-Initial");
          expected.push_back("BA::Entry");
          expected.push_back("BA::Do");

          sm.dispatch<Triggers::A_B>();
          expected.push_back("A<-B");
          expected.push_back("BA::Exit");
          expected.push_back("B::Exit");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

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
          typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake>> ToplevelInitTransition;
          typedef Statemachine<
            Toplevel_transitions,
            ToplevelInitTransition,
            NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::BB_A>();
          expected.push_back("BB<-A");
          expected.push_back("A::Exit");
          expected.push_back("B::Entry");
          expected.push_back("BB::Entry");
          expected.push_back("BBA<-Initial");
          expected.push_back("BBA::Entry");
          expected.push_back("BBA::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

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
          typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake>> ToplevelInitTransition;
          typedef Statemachine<
            Toplevel_transitions,
            ToplevelInitTransition,
            NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::BBB_A>();
          expected.push_back("BBB<-A");
          expected.push_back("A::Exit");
          expected.push_back("B::Entry");

          expected.push_back("BB::Entry");
          expected.push_back("BBB::Entry");
          expected.push_back("BBB::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());
        }

        TEST_METHOD(Callsequence_BB__BA)
        {
          typedef InitialTransition<B, StateTypeCreationPolicyType, ActionSpy<B, InitialStateFake>> ToplevelInitTransition;
          typedef Statemachine<
            Toplevel_transitions,
            ToplevelInitTransition,
            NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("B<-Initial");
          expected.push_back("B::Entry");
          expected.push_back("BA<-Initial");
          expected.push_back("BA::Entry");
          expected.push_back("BA::Do");

          sm.dispatch<Triggers::BB_BA>();
          expected.push_back("BB<-BA");
          expected.push_back("BA::Exit");
          expected.push_back("BB::Entry");
          expected.push_back("BBA<-Initial");
          expected.push_back("BBA::Entry");
          expected.push_back("BBA::Do");

          sm.dispatch<Triggers::BA_BB>();
          expected.push_back("BA<-BB");
          expected.push_back("BBA::Exit");
          expected.push_back("BB::Exit");
          expected.push_back("BA::Entry");
          expected.push_back("BA::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());
        }

        TEST_METHOD(Callsequence_A__BA)
        {
          typedef InitialTransition<B, StateTypeCreationPolicyType, ActionSpy<B, InitialStateFake>> ToplevelInitTransition;
          typedef Statemachine<
            Toplevel_transitions,
            ToplevelInitTransition,
            NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("B<-Initial");
          expected.push_back("B::Entry");
          expected.push_back("BA<-Initial");
          expected.push_back("BA::Entry");
          expected.push_back("BA::Do");

          sm.dispatch<Triggers::A_BA>();
          expected.push_back("A<-BA");
          expected.push_back("BA::Exit");
          expected.push_back("B::Exit");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());
        }

      };
    }
  }
}
