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
#include "TestHelpers.h"

#include <vector>

// Unit tests
namespace UT {
  // State machine
  namespace SM {
    // Call sequence
    namespace Hi {

      using namespace Microsoft::VisualStudio::CppUnitTestFramework;
      using namespace tsmlib;
      using namespace std;
      using namespace UnitTests::Helpers;

      struct TypeidStateComperator {};

      typedef State<VirtualGetTypeIdStateComperator, false> StateType;
      typedef FactorCreatorFake<StateType> StateTypeCreationPolicyType;
      template<typename Derived> struct Leaf : BasicState<Derived, StateType>, FactorCreatorFake<Derived> {};
      template<typename Derived, typename Statemachine> struct Composite : SubstatesHolderState<Derived, StateType, Statemachine>, FactorCreatorFake<Derived> {};

      struct InitialStateFake : StateType {
        static const char* Name;
      };
      const char* InitialStateFake::Name = "Initial";

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
        B_A,
        A_B_HIST,
        AB_AA,
      };

      vector<string> recorder;

      struct AA;
      struct AB;

      typedef Transition<Triggers::AB_AA, AB, AA, StateTypeCreationPolicyType, OkGuard, ActionSpy<AB, AA>> AB_AA_t;
      typedef
        Typelist<AB_AA_t,
        NullType> A_transitions;

      typedef InitialTransition<AA, StateTypeCreationPolicyType, ActionSpy<AA, InitialStateFake>> A_initt;
      typedef Statemachine<
        A_transitions,
        A_initt,
        NullEndTransition<StateTypeCreationPolicyType>> A_sm;

      struct A : Composite<A, A_sm> {
        static const char* Name;
        void entry() { recorder.push_back("A::Entry"); }
        void exit() { recorder.push_back("A::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("A::Do"); }
        uint8_t getTypeId() const override { return 1; }
      };
      const char* A::Name = "A";

      struct AA : Leaf<AA> {
        static const char* Name;
        void entry() { recorder.push_back("AA::Entry"); }
        void exit() { recorder.push_back("AA::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("AA::Do"); }
        uint8_t getTypeId() const override { return 11; }
      };
      const char* AA::Name = "AA";

      struct AB : Leaf<AB> {
        static const char* Name;
        void entry() { recorder.push_back("AB::Entry"); }
        void exit() { recorder.push_back("AB::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("AB::Do"); }
        uint8_t getTypeId() const override { return 12; }
      };
      const char* AB::Name = "AB";

      struct B : Leaf<B> {
        static const char* Name;
        void entry() { recorder.push_back("B::Entry"); }
        void exit() { recorder.push_back("B::Exit"); }
        template<uint8_t N>
        void doit() { recorder.push_back("B::Do"); }
        uint8_t getTypeId() const override { return 2; };
      };
      const char* B::Name = "B";

      typedef Transition<Triggers::B_A, B, A, StateTypeCreationPolicyType, OkGuard, ActionRecordingSpy<B, A>> B_A_t;
      typedef Transition<Triggers::A_B_HIST, A, B, StateTypeCreationPolicyType, OkGuard, ActionRecordingSpy<A, B>> A_B_t;
      typedef
        Typelist<B_A_t,
        Typelist<A_B_t,
        NullType>> ToplevelTransitions;

      typedef InitialTransition<A, StateTypeCreationPolicyType, ActionRecordingSpy<A, InitialStateNamedFake<StateType>>> ToplevelInitTransition;
      typedef Statemachine<
        ToplevelTransitions,
        ToplevelInitTransition,
        NullEndTransition<StateTypeCreationPolicyType>> Toplevel_sm;

      TEST_CLASS(HistoryTransitionTest)
      {
      public:

        TEST_METHOD_INITIALIZE(Initialize)
        {
          FactorCreatorFake<A>::reset();
          FactorCreatorFake<AA>::reset();
          FactorCreatorFake<AB>::reset();
          FactorCreatorFake<B>::reset();
        }

        TEST_METHOD(Statemachine_Initialize)
        {
          Toplevel_sm sm;
          vector<string> expected;
          recorder.clear();

          sm.begin();
          expected.push_back("A<-Initial");
          expected.push_back("A::Entry");
          expected.push_back("AA::Entry");
          expected.push_back("AA::Do");

          Assert::IsTrue(expected.size() >= recorder.size());
          for (int n = 0; n < recorder.size(); n++) {
            string exp = expected[n];
            string rec = recorder[n];
            Assert::AreEqual<string>(exp, rec);
          }
          Assert::AreEqual<size_t>(expected.size(), recorder.size());

          // Active state is B
          Assert::AreEqual<int>(FactorCreatorFake<A>::CreateCalls, FactorCreatorFake<A>::DeleteCalls + 1);
          Assert::AreEqual<int>(FactorCreatorFake<AA>::CreateCalls, FactorCreatorFake<AA>::DeleteCalls + 1);
          Assert::AreEqual<int>(FactorCreatorFake<AB>::CreateCalls, FactorCreatorFake<AB>::DeleteCalls);
          Assert::AreEqual<int>(FactorCreatorFake<B>::CreateCalls, FactorCreatorFake<B>::DeleteCalls);
        }

      };
    }
  }
}
