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

// Unit tests
namespace UT {
  // State machine
  namespace SM {
    // Call sequence
    namespace IF {

      using namespace Microsoft::VisualStudio::CppUnitTestFramework;
      using namespace tsmlib;
      using namespace std;
      using namespace UnitTests::Helpers;

      typedef State<MemoryAddressStateComperator<true>, true> StateType;
      typedef SingletonCreatorFake<StateType> StateTypeCreationPolicyType;
      template<typename Derived> struct Leaf : BasicState<Derived, StateType>, SingletonCreatorFake<Derived> {};
      template<typename Derived, typename Statemachine> struct Composite : SubstatesHolderState<Derived, StateType, Statemachine>, SingletonCreatorFake<Derived> {};

      struct InitialStateFake : StateType {
        static const char* Name;
      };
      const char* InitialStateFake::Name = "Initial";

      template<typename T>
      struct EmptyStateFake : T {
        static const char* Name;

        typedef EmptyStateFake CreatorType;
        typedef EmptyStateFake ObjectType;

        static EmptyStateFake* create() {
          return 0;
        }
        static void destroy(EmptyStateFake*) { }

        template<uint8_t N>
        bool _entry() { return false; }
        template<uint8_t N>
        EmptyStateFake* _doit() { return 0; }
      };
      template<typename T> const char* EmptyStateFake<T>::Name = "Final";

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
        Finalize,
        BB_BA,
      };

      vector<string> recorder;

      struct A;
      struct B;
      struct BA;
      struct BB;
      struct BBA;
      struct BBB;

      struct BBAEndGuard {
        template<typename T> bool eval(T*) {
          return evalResult;
        }
        static bool evalResult;
      };
      bool BBAEndGuard::evalResult = false;

      struct A : Leaf<A> {
        static const char* Name;
        void entry() { recorder.push_back("A::Entry"); }
        void exit() { recorder.push_back("A::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("A::Do"); }
      };
      const char* A::Name = "A";

      typedef Transition<Triggers::Finalize, EmptyStateFake<StateType>, BA, StateTypeCreationPolicyType, OkGuard, ActionSpy<EmptyStateFake<StateType>, BA>> F_BA_t;
      typedef Transition<Triggers::BB_BA, BB, BA, StateTypeCreationPolicyType, OkGuard, ActionSpy<BB, BA>> BB_BA_t;
      typedef ExitDeclaration<Triggers::Finalize, EmptyStateFake<StateType>, BB, StateTypeCreationPolicyType, BBAEndGuard> Finalize_BB_decl;
      typedef
        Typelist<F_BA_t,
        Typelist<BB_BA_t,
        Typelist<Finalize_BB_decl,
        NullType>>> B_transitions;

      typedef InitialTransition<BA, StateTypeCreationPolicyType, ActionSpy<BA, InitialStateFake>> B_initt;
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

      typedef
        Typelist<NullTransition<StateTypeCreationPolicyType>,
        NullType> BB_transitions;

      typedef InitialTransition<BBA, StateTypeCreationPolicyType, ActionSpy<BBA, InitialStateFake>> BB_initt;
      typedef EndTransition<StateTypeCreationPolicyType, BBAEndGuard, ActionSpy<EmptyStateFake<StateType>, BBA>> A__BBA_endt;
      typedef Statemachine<
        BB_transitions,
        BB_initt,
        A__BBA_endt> BB_sm;

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

      typedef Transition<Triggers::Finalize, EmptyStateFake<StateType>, A, StateTypeCreationPolicyType, OkGuard, ActionSpy<EmptyStateFake<StateType>, A>> F_A_t;
      typedef Transition<Triggers::Finalize, EmptyStateFake<StateType>, B, StateTypeCreationPolicyType, OkGuard, ActionSpy<EmptyStateFake<StateType>, B>> F_B_t;
      typedef Declaration<Triggers::BB_BA, B, StateTypeCreationPolicyType> BB_BA_decl;
      typedef ExitDeclaration<Triggers::Finalize, A, B, StateTypeCreationPolicyType, BBAEndGuard> Finalize_B_decl;
      typedef
        Typelist< F_A_t,
        Typelist< F_B_t,
        Typelist<BB_BA_decl,
        Typelist<Finalize_B_decl,
        NullType>>>> Toplevel_transitions;

      TEST_CLASS(InitialAndFinalTransitionTest)
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

          BBAEndGuard::evalResult = false;
        }

        TEST_METHOD(Callsequence_A__Initial)
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

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is A
          Assert::AreNotEqual<int>(0, SingletonCreatorFake<A>::createCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, SingletonCreatorFake<BA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
        }

        TEST_METHOD(Callsequence_B__Initial)
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

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is B/BA
          Assert::AreNotEqual<int>(0, SingletonCreatorFake<B>::createCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, SingletonCreatorFake<BA>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
        }

        TEST_METHOD(Callsequence_Final__A)
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

          sm.dispatch<Triggers::Finalize>();
          expected.push_back("Final<-A");
          expected.push_back("A::Exit");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is Final
          Assert::AreNotEqual<int>(0, SingletonCreatorFake<A>::createCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, SingletonCreatorFake<BA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
        }

        TEST_METHOD(Callsequence_Final__B)
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

          sm.dispatch<Triggers::Finalize>();
          expected.push_back("Final<-B");
          expected.push_back("BA::Exit");
          expected.push_back("B::Exit");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is Final
          Assert::AreNotEqual<int>(0, SingletonCreatorFake<B>::createCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
          // Finalize is called and the sub-state is exited using polymorphism. StateTypeCreationPolicyType is used to delete the state.
          Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, StateTypeCreationPolicyType::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
        }

        TEST_METHOD(Callsequence_Final__BA)
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

          sm.dispatch<Triggers::Finalize>();
          expected.push_back("Final<-B");
          expected.push_back("BA::Exit");
          expected.push_back("B::Exit");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is Final
          Assert::AreNotEqual<int>(0, SingletonCreatorFake<B>::createCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
          // Finalize is called and the sub-state is exited using polymorphism. StateTypeCreationPolicyType is used to delete the state.
          Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, StateTypeCreationPolicyType::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);
        }

        TEST_METHOD(Callsequence_A__Final_BBA)
        {
          NullEndTransition< StateTypeCreationPolicyType> NET;
          EndTransition< StateTypeCreationPolicyType, BBAEndGuard, EmptyAction> ET;

          typedef InitialTransition<B, StateTypeCreationPolicyType, ActionSpy<B, InitialStateFake>> ToplevelInitTransition;
          typedef Statemachine<
            Toplevel_transitions,
            ToplevelInitTransition,
            NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          BBAEndGuard::evalResult = true;

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

          // Active state is B/BB/BBA
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<BA>::createCalls, SingletonCreatorFake<BA>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<BB>::createCalls, SingletonCreatorFake<BB>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<BBA>::createCalls, SingletonCreatorFake<BBA>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<BBB>::createCalls, SingletonCreatorFake<BBB>::deleteCalls);

          sm.dispatch<Triggers::Finalize>();
          expected.push_back("Final<-BBA");
          expected.push_back("BBA::Exit");
          expected.push_back("BB::Exit");
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
