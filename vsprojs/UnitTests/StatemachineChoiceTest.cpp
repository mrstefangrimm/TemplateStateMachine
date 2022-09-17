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
#include "..\..\src\choicetransition.h"
#include "..\..\src\choice.h"
#include "TestHelpers.h"

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
      using namespace UnitTests::Helpers;

      typedef State<VirtualGetTypeIdStateComperator, false> StateType;
      typedef FactorCreatorFake<StateType> StateTypeCreationPolicyType;
      template<typename Derived> struct Leaf : SimpleState<Derived, StateType>, FactorCreatorFake<Derived> {};
      template<typename Derived, typename Statemachine> struct Composite : SubstatesHolderState<Derived, StateType, Statemachine>, FactorCreatorFake<Derived> {};

      template<typename To, typename From>
      struct ActionRecordingSpy {
        template<typename T>
        void perform(T*) {
          ostringstream buf;
          buf << To::Name << "<-" << From::Name;
          recorder.push_back(buf.str());
        }
      };

      enum Triggers {
        Choice_B_C,
        Choice_A,
      };

      vector<string> recorder;

      struct A : Leaf<A> {
        static const char* Name;
        void entry() { recorder.push_back("A::Entry"); }
        void exit() { recorder.push_back("A::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("A::Do"); }

        uint8_t getTypeId() const override { return 1; };
      };
      const char* A::Name = "A";

      struct B : Leaf<B> {
        static const char* Name;
        void entry() { recorder.push_back("B::Entry"); }
        void exit() { recorder.push_back("B::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("B::Do"); }

        uint8_t getTypeId() const override { return 2; };
      };
      const char* B::Name = "B";

      struct C : Leaf<C> {
        static const char* Name;
        void entry() { recorder.push_back("C::Entry"); }
        void exit() { recorder.push_back("C::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("C::Do"); }

        uint8_t getTypeId() const override { return 3; };
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
        bool eval(T*) {
          return ReturnValue;
        }
      };
      bool ChoiceGuard::ReturnValue = true;

      typedef ChoiceTransition<Triggers::Choice_B_C, B, C, A, StateTypeCreationPolicyType, ChoiceGuard, ActionRecordingSpy<X, A>> A_B_t;
      typedef ChoiceTransition<Triggers::Choice_A, B, A, A, StateTypeCreationPolicyType, ChoiceGuard, ActionRecordingSpy<X, A>> A_A_t;
      typedef
        Typelist< A_B_t,
        Typelist< A_A_t,
        NullType>> Transitions;

      typedef InitialTransition<A, StateTypeCreationPolicyType, ActionRecordingSpy<A, InitialStateNamedFake<StateType>>> ToplevelInitTransition;
      typedef Statemachine<
        Transitions,
        ToplevelInitTransition,
        NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

      TEST_CLASS(StatemachineChoiceTests)
      {
      public:

        TEST_METHOD_INITIALIZE(Initialize)
        {
          FactorCreatorFake<A>::reset();
          FactorCreatorFake<B>::reset();
          FactorCreatorFake<C>::reset();
        }

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

          // Active state is B
          Assert::AreNotEqual<int>(0, FactorCreatorFake<B>::CreateCalls);
          Assert::AreEqual<int>(FactorCreatorFake<A>::CreateCalls, FactorCreatorFake<A>::DeleteCalls);
          Assert::AreEqual<int>(FactorCreatorFake<B>::CreateCalls, FactorCreatorFake<B>::DeleteCalls + 1);
          Assert::AreEqual<int>(FactorCreatorFake<C>::CreateCalls, FactorCreatorFake<C>::DeleteCalls);
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

          // Active state is C
          Assert::AreNotEqual<int>(0, FactorCreatorFake<C>::CreateCalls);
          Assert::AreEqual<int>(FactorCreatorFake<A>::CreateCalls, FactorCreatorFake<A>::DeleteCalls);
          Assert::AreEqual<int>(FactorCreatorFake<B>::CreateCalls, FactorCreatorFake<B>::DeleteCalls);
          Assert::AreEqual<int>(FactorCreatorFake<C>::CreateCalls, FactorCreatorFake<C>::DeleteCalls + 1);
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

          // Active state is A
          Assert::AreNotEqual<int>(0, FactorCreatorFake<A>::CreateCalls);
          Assert::AreEqual<int>(FactorCreatorFake<A>::CreateCalls, FactorCreatorFake<A>::DeleteCalls + 1);
          Assert::AreEqual<int>(FactorCreatorFake<B>::CreateCalls, FactorCreatorFake<B>::DeleteCalls);
          Assert::AreEqual<int>(FactorCreatorFake<C>::CreateCalls, FactorCreatorFake<C>::DeleteCalls);
        }

      };
    }
  }
}
