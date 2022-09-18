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
#include "..\..\src\initialtransition.h"
#include "..\..\src\endtransition.h"

#include <vector>

namespace UnitTests {

  namespace Recording {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace std;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;
    typedef FactorCreator<StateType, false> StateTypeCreationPolicyType;

    struct InitialStateFake : StateType {
      static const char* Name;
    };
    const char* InitialStateFake::Name = "Initial";

    struct FinalStateFake : SimpleState<FinalStateFake, StateType> {
      static const char* Name;
      typedef FinalStateFake CreatorType;
      typedef FinalStateFake ObjectType;
      static FinalStateFake* create() { return 0; }
      static void destroy(FinalStateFake*) { }

    private:
      friend class SimpleState<FinalStateFake, StateType>;
      void entry() { }
      void exit() { }
      template<uint8_t N>
      void doit() { }
    };
    const char* FinalStateFake::Name = "Final";

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
      On,
      Off,
      OnToOn,
      OffToOff,
      OffToFinal
    };

    vector<string> recorder;

    struct OnState : SimpleState<OnState, StateType>, FactorCreator<OnState> {
      static const char* Name;
      uint8_t getTypeId() const override { return 1; }

    private:
      friend class SimpleState<OnState, StateType>;
      void entry() { recorder.push_back("OnState::Entry"); }
      void exit() { recorder.push_back("OnState::Exit"); }
      template<uint8_t N>
      void doit() { recorder.push_back("OnState::Do"); }
    };
    const char* OnState::Name = "OnState";

    struct OffState : SimpleState<OffState, StateType>, FactorCreator<OffState> {
      static const char* Name;
      uint8_t getTypeId() const override { return 2; }

    private:
      friend class SimpleState<OffState, StateType>;
      void entry() { recorder.push_back("OffState::Entry"); }
      void exit() { recorder.push_back("OffState::Exit"); }
      template<uint8_t N>
      void doit() { recorder.push_back("OffState::Do"); }
    };
    const char* OffState::Name = "OffState";

    typedef ActionSpy<struct OnState, struct OffState> ToOnFromOffActionSpy;
    typedef ActionSpy<struct OffState, struct OnState> ToOffFromOnActionSpy;
    typedef ActionSpy<struct OnState, struct OnState> ToOnFromOnActionSpy;
    typedef ActionSpy<struct OffState, struct OffState> ToOffFromOffActionSpy;
    typedef ActionSpy<struct FinalStateFake, struct OffState> ToFinalFromOffActionSpy;

    typedef Transition<Triggers::On, OnState, OffState, StateTypeCreationPolicyType, OkGuard, ToOnFromOffActionSpy> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, OffState, OnState, StateTypeCreationPolicyType, OkGuard, ToOffFromOnActionSpy> ToOffFromOnTransition;
    typedef SelfTransition<Triggers::OnToOn, OnState, StateTypeCreationPolicyType, OkGuard, ToOnFromOnActionSpy> ToOnFromOnTransition;
    typedef SelfTransition<Triggers::OffToOff, OffState, StateTypeCreationPolicyType, OkGuard, ToOffFromOffActionSpy> ToOffFromOffTransition;
    typedef Transition<Triggers::OffToFinal, FinalStateFake, OffState, StateTypeCreationPolicyType, OkGuard, ToFinalFromOffActionSpy> ToFinalFromOffTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToOnFromOnTransition,
      Typelist<ToOffFromOffTransition,
      Typelist<ToFinalFromOffTransition,
      NullType>>>>> TransitionList;

    typedef ActionSpy<struct OffState, struct InitialStateFake> ToOffFromInitialActionSpy;
    typedef InitialTransition<OffState, StateTypeCreationPolicyType, ToOffFromInitialActionSpy> InitTransition;
    typedef Statemachine<
      TransitionList,
      InitTransition,
      NullEndTransition<StateTypeCreationPolicyType>> Sm;

    TEST_CLASS(StatemachineOnOffCallSequence)
    {
    public:

      TEST_METHOD(EntriesDoesExits_Roundtrip)
      {
        OnState on;
        OffState off;

        vector<string> expected;

        Sm sm;
        sm.begin();
        expected.push_back("OffState<-Initial");
        expected.push_back("OffState::Entry");
        expected.push_back("OffState::Do");

        // Off <- Off, self transition
        auto result = sm.dispatch<Triggers::OffToOff>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        expected.push_back("OffState<-OffState");
        expected.push_back("OffState::Do");

        // Off <- Off, unhandled trigger
        result = sm.dispatch<Triggers::Off>();

        // On <- Off
        result = sm.dispatch<Triggers::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        expected.push_back("OnState<-OffState");
        expected.push_back("OffState::Exit");
        expected.push_back("OnState::Entry");
        expected.push_back("OnState::Do");

        // On <- On, self transition
        result = sm.dispatch<Triggers::OnToOn>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());
        expected.push_back("OnState<-OnState");
        expected.push_back("OnState::Do");

        // On <- On, unhandled trigger
        result = sm.dispatch<Triggers::On>();
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());

        // Off <- On, unhandled trigger
        result = sm.dispatch<Triggers::Off>();
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());
        expected.push_back("OffState<-OnState");
        expected.push_back("OnState::Exit");
        expected.push_back("OffState::Entry");
        expected.push_back("OffState::Do");

        result = sm.dispatch<Triggers::OffToFinal>();
        expected.push_back("Final<-OffState");
        expected.push_back("OffState::Exit");

        Assert::AreEqual<size_t>(expected.size(), recorder.size());
        for (int n = 0; n < recorder.size(); n++) {
          string exp = expected[n];
          string rec = recorder[n];
          Assert::AreEqual<string>(exp, rec);
        }
      }
    };
  }
}
