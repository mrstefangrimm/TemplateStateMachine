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
#include "..\..\src\choice.h"

#include <vector>

// Unit tests
namespace UT {
  // State machine
  namespace SM {
    // Call sequence
    namespace Ch {

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
        Choice_B_C,
        Choice_A,
      };

      vector<string> recorder;

      struct AA : Leaf<AA> {
        static const char* Name;
        void entry() { recorder.push_back("AA::Entry"); }
        void exit() { recorder.push_back("AA::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("AA::Do"); }
      };
      const char* AA::Name = "AA";

      struct AB : Leaf<AB> {
        static const char* Name;
        void entry() { recorder.push_back("AB::Entry"); }
        void exit() { recorder.push_back("AB::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("AB::Do"); }
      };
      const char* AB::Name = "AB";


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

      struct X : Leaf<X> {
        static const char* Name;
        void entry() { recorder.push_back("X::Entry"); }
        void exit() { recorder.push_back("X::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("X::Do"); }
      };
      const char* X::Name = "X";

      struct ChoiceGuard {
        static bool ReturnValue;
        template<typename T>
        bool check(T*) {
          return ReturnValue;
        }
      };
      bool ChoiceGuard::ReturnValue = true;

      typedef ChoiceTransition<Triggers::Choice_B_C, B, C, A, StateTypeCreationPolicyType, ChoiceGuard, ActionSpy<X, A>> A_B_t;
      typedef ChoiceTransition<Triggers::Choice_A, B, A, A, StateTypeCreationPolicyType, ChoiceGuard, ActionSpy<X, A>> A_A_t;
      typedef
        Typelist< A_B_t,
        Typelist< A_A_t,
        NullType>> Transitions;

      typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake>> ToplevelInitTransition;
      typedef Statemachine<
        Transitions,
        ToplevelInitTransition,
        NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

      TEST_CLASS(StatemachineChoiceTests)
      {
      public:

        TEST_METHOD(Callsequence_B__A)
        {
          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();
          ChoiceGuard::ReturnValue = true;

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::Choice_B_C>();
          expected.push_back("X<-A");
          expected.push_back("A::Exit");
          expected.push_back("B::Entry");
          expected.push_back("B::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());
        }

        TEST_METHOD(Callsequence_C__A)
        {
          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();
          ChoiceGuard::ReturnValue = false;

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::Choice_B_C>();
          expected.push_back("X<-A");
          expected.push_back("A::Exit");
          expected.push_back("C::Entry");
          expected.push_back("C::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());
        }

        TEST_METHOD(Callsequence_A__A)
        {
          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();
          ChoiceGuard::ReturnValue = false;

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("A::Do");

          sm.dispatch<Triggers::Choice_A>();
          expected.push_back("X<-A");
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
