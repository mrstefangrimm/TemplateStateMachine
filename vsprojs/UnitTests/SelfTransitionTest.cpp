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
    // Self
    namespace SL {

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
        B_A,
        C_B,
        Self,
        Reenter,
      };

      vector<string> recorder;

      struct A : Leaf<A> {
        static const char* Name;
        void entry() { recorder.push_back("A::Entry"); }
        void exit() { recorder.push_back("A::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("A::Do"); }
      };
      const char* A::Name = "A";

      struct B : Leaf<B> {
        static const char* Name;
        void entry() { recorder.push_back("B::Entry"); }
        void exit() { recorder.push_back("B::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("B::Do"); }
      };
      const char* B::Name = "B";

      struct C : Leaf<C> {
        static const char* Name;
        void entry() { recorder.push_back("C::Entry"); }
        void exit() { recorder.push_back("C::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("C::Do"); }
      };
      const char* C::Name = "C";

      typedef Transition<Triggers::B_A, B, A, StateTypeCreationPolicyType, OkGuard, ActionSpy<B, A>> B_A_t;
      typedef Transition<Triggers::C_B, C, B, StateTypeCreationPolicyType, OkGuard, ActionSpy<C, B>> C_B_t;
      typedef SelfTransition<Triggers::Reenter, A, StateTypeCreationPolicyType, OkGuard, ActionSpy<A, A>, true> A_A_rt;
      typedef SelfTransition<Triggers::Reenter, B, StateTypeCreationPolicyType, OkGuard, ActionSpy<B, B>, true> B_B_rt;
      typedef SelfTransition<Triggers::Reenter, C, StateTypeCreationPolicyType, OkGuard, ActionSpy<C, C>, true> C_C_rt;
      typedef SelfTransition<Triggers::Self, A, StateTypeCreationPolicyType, OkGuard, ActionSpy<A, A>> A_A_t;
      typedef SelfTransition<Triggers::Self, B, StateTypeCreationPolicyType, OkGuard, ActionSpy<B, B>> B_B_t;
      typedef SelfTransition<Triggers::Self, C, StateTypeCreationPolicyType, OkGuard, ActionSpy<C, C>> C_C_t;

      typedef
        Typelist<B_A_t,
        Typelist<C_B_t,
        Typelist<A_A_rt,
        Typelist<B_B_rt,
        Typelist<C_C_rt,
        Typelist<A_A_t,
        Typelist<B_B_t,
        Typelist<C_C_t,
        NullType>>>>>>>> Transitions;

      typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake>> ToplevelInitTransition;
      typedef Statemachine<
        Transitions,
        ToplevelInitTransition,
        NullEndTransition<StateTypeCreationPolicyType>> Sm;

      TEST_CLASS(SelfTransitionTest)
      {
      public:

        TEST_METHOD_INITIALIZE(Initialize)
        {
          SingletonCreatorFake<A>::reset();
          SingletonCreatorFake<B>::reset();
          SingletonCreatorFake<C>::reset();
        }

        TEST_METHOD(Self_StateA)
        {
          Sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::Self>();
          expected.push_back("A<-A");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::Reenter>();
          expected.push_back("A<-A");
          expected.push_back("A::Exit");
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
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
        }

        TEST_METHOD(Self_StateB)
        {
          Sm sm;
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
          expected.push_back("B::Do");

          sm.dispatch<Triggers::Self>();
          expected.push_back("B<-B");
          expected.push_back("B::Do");

          sm.dispatch<Triggers::Reenter>();
          expected.push_back("B<-B");
          expected.push_back("B::Exit");
          expected.push_back("B::Entry");
          expected.push_back("B::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is B
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
          Assert::AreEqual<int>(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
        }

        TEST_METHOD(Self_StateC)
        {
          Sm sm;
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
          expected.push_back("B::Do");

          sm.dispatch<Triggers::C_B>();
          expected.push_back("C<-B");
          expected.push_back("B::Exit");
          expected.push_back("C::Entry");
          expected.push_back("C::Do");

          sm.dispatch<Triggers::Self>();
          expected.push_back("C<-C");
          expected.push_back("C::Do");

          sm.dispatch<Triggers::Reenter>();
          expected.push_back("C<-C");
          expected.push_back("C::Exit");
          expected.push_back("C::Entry");
          expected.push_back("C::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is C
          Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
          Assert::AreEqual<int>(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls + 1);
        }

      };
    }
  }
}
