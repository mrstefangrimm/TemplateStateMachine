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

#include "..\..\src\state.h"
#include "..\..\src\templatemeta.h"
#include "..\..\src\statemachine.h"
#include "..\..\src\transition.h"

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

      typedef State<MemoryAddressStateComperator<true>, true> StateType;
      typedef SingletonCreator<StateType> StateTypeCreationPolicyType;
      template<typename Derived> struct Leaf : SimpleState<Derived, StateType>, SingletonCreator<Derived> {};
      template<typename Derived, typename Statemachine> struct Composite : SubstatesHolderState<Derived, StateType, Statemachine>, SingletonCreator<Derived> {};

      struct InitialStateFake : StateType {
        static const char* Name;
      };
      const char* InitialStateFake::Name = "Initial";

      struct AnyStateFake : SimpleState<AnyStateFake, StateType>, StateTypeCreationPolicyType {
        static const char* Name;
        void entry() { }
        void exit() { }
        template<uint8_t N>
        void doit() { }
      };
      const char* AnyStateFake::Name = "AnyStateFake";

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
        B_A,
        AA_AB,
        AB_AA,
        BA_BB,
        BB_BA,
        BB_AA,
        BA_AA,
        B_AA,
        B_AB,
        BB_AB,
        BB_A,
        BBB_A,
      };

      vector<string> recorder;

      typedef ActionSpy<struct B, struct A> B_A_spy;
      typedef ActionSpy<struct A, struct B> A_B_spy;
      typedef ActionSpy<struct BB, struct BA> BB_BA_spy;
      typedef ActionSpy<struct BA, struct BB> BA_BB_spy;
      typedef ActionSpy<struct BB, struct A> BB_A_spy;
      typedef ActionSpy<struct BBB, struct A> BBB_A_spy;
      typedef ActionSpy<struct BB, struct B> BB_B_spy;
      typedef ActionSpy<struct BBB, struct B> BBB_B_spy;

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
      typedef EntryDeclaration<Triggers::BB_A, BB, StateTypeCreationPolicyType, BB_B_spy> BB_B_t;
      //typedef EntryDeclaration<Triggers::BBB_A, BB, StateTypeCreationPolicyType, BBB_B_spy> BBB_B_t;
      typedef
        Typelist<BB_BA_t,
        Typelist<BA_BB_t,
        Typelist<BB_B_t,
        //Typelist<BBB_B_t,
        NullType>>> B_transitions;
      typedef ActionSpy<BA, InitialStateFake> B_Init_spy;
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

      typedef ActionSpy<BBA, BB> BBB_BB_spy;
      typedef EntryDeclaration<Triggers::BBB_A, BB, StateTypeCreationPolicyType, BBB_BB_spy> BBB_BB_t;
      typedef
        Typelist<BBB_BB_t,
        NullType> BB_transitions;
      typedef ActionSpy<BBA, InitialStateFake> BB_Init_spy;
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


      typedef Transition<Triggers::B_A, B, A, StateTypeCreationPolicyType, OkGuard, B_A_spy> B_A_t;
      typedef Transition<Triggers::A_B, A, B, StateTypeCreationPolicyType, OkGuard, A_B_spy> A_B_t;
      //typedef EntryTransformation<Triggers::BB_A, B, A, StateTypeCreationPolicyType, BB_A_spy> BB_A_t;
      typedef Transition<Triggers::BB_A, B, A, StateTypeCreationPolicyType, OkGuard, BB_A_spy> BB_A_t;
      typedef Transition<Triggers::BBB_A, B, A, StateTypeCreationPolicyType, OkGuard, BBB_A_spy> BBB_A_t;
      typedef
        Typelist<B_A_t,
        Typelist<A_B_t,
        Typelist<BB_A_t,
        Typelist<BBB_A_t,
        NullType>>>> Toplevel_transitions;
      typedef ActionSpy<A, InitialStateFake> Toplevel_Init_spy;
      typedef InitialTransition<A, StateTypeCreationPolicyType, Toplevel_Init_spy> ToplevelInitTransition;
      typedef Statemachine<
        Toplevel_transitions,
        ToplevelInitTransition,
        NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

      //// sub-states transitions are self transitions
      //typedef Declaration<Triggers::On, ActiveState, StateTypeCreationPolicyType> ToOnFromOffSubTransition;
      //typedef Declaration<Triggers::Off, ActiveState, StateTypeCreationPolicyType> ToOffFromOnSubTransition;
      //typedef ExitDeclaration<Triggers::GoodbyeSub, IdleState, ActiveState, StateTypeCreationPolicyType> ToIdleFromOffSubTransition;
      //typedef ActionSpy<struct ActiveState, struct IdleState> ToActiveFromIdleActionSpy;
      //typedef ActionSpy<struct IdleState, struct AnyStateFake> ToIdleFromAnyActionSpy;
      //typedef Transition<Triggers::Hello, ActiveState, IdleState, StateTypeCreationPolicyType, OkGuard, ToActiveFromIdleActionSpy> ToActiveFromIdleTransition;
      ////typedef Transition<Triggers::Goodbye, StateType, IdleState, AnyState<StateType>, OkGuard, ToIdleFromAnyActionSpy> ToIdleFromActiveTransition;
      //typedef
      //  Typelist<ToOnFromOffSubTransition,
      //  Typelist<ToOffFromOnSubTransition,
      //  Typelist<ToIdleFromOffSubTransition,
      //  Typelist<ToActiveFromIdleTransition,
      //  //Typelist<ToIdleFromActiveTransition,
      //  NullType>>>> TransitionList;
      //struct ActiveStateFinalizeGuard {
      //  template<typename T>
      //  bool check(T*) { return true; }
      //typedef ActionSpy<struct IdleState, struct InitialStateFake> ToIdleFromInitialActionSpy;
      //typedef InitialTransition<IdleState, StateTypeCreationPolicyType, ToIdleFromInitialActionSpy> InitTransition;
      //typedef Statemachine<
      //  TransitionList,
      //  InitTransition,
      //  NullEndTransition<StateTypeCreationPolicyType>> Sm;

      TEST_CLASS(StatemachineCallSequenceTests)
      {
      public:

        TEST_METHOD(Callsequence_Toplevel)
        {
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
        }

        TEST_METHOD(Callsequence_TopToFirstLevel)
        {
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
          expected.push_back("BB::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());
        }

        TEST_METHOD(Callsequence_TopToSecondLevel)
        {
          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::BBB_A>();
          expected.push_back("BB<-A");
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

      };
    }
  }
}
