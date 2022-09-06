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
#include "CppUnitTest.h"

#include "tsmlib/state.h"
#include "tsmlib/templatemeta.h"
#include "tsmlib/statemachine.h"
#include "tsmlib/transition.h"

#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace tsmlib;
using namespace std;

namespace UnitTests {

  namespace Recording {

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;

    struct InitialStateFake : StateType {
      static const char* Name;
    };
    const char* InitialStateFake::Name = "Initial";

    struct FinalStateFake : StateType {
      static const char* Name;
      typedef FinalStateFake CreatorType;
      void entry() { }
      void doit() { }
      static FinalStateFake* Create() { return 0; }
      static void Delete(FinalStateFake*) { }
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
    typedef ActionSpy<struct OffState, struct InitialStateFake> ToOffFromInitialActionSpy;
    typedef ActionSpy<struct OnState, struct OffState> ToOnFromOffActionSpy;
    typedef ActionSpy<struct OffState, struct OnState> ToOffFromOnActionSpy;
    typedef ActionSpy<struct OnState, struct OnState> ToOnFromOnActionSpy;
    typedef ActionSpy<struct OffState, struct OffState> ToOffFromOffActionSpy;
    typedef ActionSpy<struct FinalStateFake, struct OffState> ToFinalFromOffActionSpy;

    enum Triggers {
      On,
      Off,
      OnToOn,
      OffToOff,
      OffToFinal
    };

    vector<string> recorder;

    struct OnState : StateType, FactorCreator<OnState> {
      static const char* Name;
      uint8_t getTypeId() const override { return 1; }
      void entry() { recorder.push_back("OnState::Entry"); }
      void exit() { recorder.push_back("OnState::Exit"); }
      void doit() { recorder.push_back("OnState::Do"); }
    };
    const char* OnState::Name = "OnState";

    struct OffState : StateType, FactorCreator<OffState> {
      static const char* Name;
      uint8_t getTypeId() const override { return 2; }
      void entry() { recorder.push_back("OffState::Entry"); }
      void exit() { recorder.push_back("OffState::Exit"); }
      void doit() { recorder.push_back("OffState::Do"); }
    };
    const char* OffState::Name = "OffState";

    typedef Transition<Triggers::On, StateType, OnState, OffState, EmptyGuard, ToOnFromOffActionSpy> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, StateType, OffState, OnState, EmptyGuard, ToOffFromOnActionSpy> ToOffFromOnTransition;
    typedef Transition<Triggers::OnToOn, StateType, OnState, OnState, EmptyGuard, ToOnFromOnActionSpy> ToOnFromOnTransition;
    typedef Transition<Triggers::OffToOff, StateType, OffState, OffState, EmptyGuard, ToOffFromOffActionSpy> ToOffFromOffTransition;
    typedef Transition<Triggers::OffToFinal, StateType, FinalStateFake, OffState, EmptyGuard, ToFinalFromOffActionSpy> ToFinalFromOffTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToOnFromOnTransition,
      Typelist<ToOffFromOffTransition,
      Typelist<ToFinalFromOffTransition,
      NullType>>>>> TransitionList;

    typedef InitialTransition<StateType, OffState, ToOffFromInitialActionSpy> InitTransition;
    typedef Statemachine<StateType, TransitionList, NullStatemachine<StateType>, InitTransition, NullFinalTransition<StateType>> SM;

    TEST_CLASS(StatemachineOnOffCallSequence)
    {
    public:

      TEST_METHOD(EntriesDoesExits_Roundtrip)
      {
        OnState on;
        OffState off;

        vector<string> expected;

        SM sm(true);
        expected.push_back("OffState<-Initial");
        expected.push_back("OffState::Entry");
        expected.push_back("OffState::Do");

        // Off <- Off, internal transition
        StateType* state = sm.trigger<Triggers::OffToOff>();
        Assert::AreEqual<int>(off.getTypeId(), state->getTypeId());
        expected.push_back("OffState<-OffState");
        expected.push_back("OffState::Do");

        // Off <- Off, unhandled trigger
        state = sm.trigger<Triggers::Off>();

        // On <- Off
        state = sm.trigger<Triggers::On>();
        Assert::AreEqual<int>(on.getTypeId(), state->getTypeId());
        expected.push_back("OffState::Exit");
        expected.push_back("OnState<-OffState");
        expected.push_back("OnState::Entry");
        expected.push_back("OnState::Do");

        // On <- On, internal transition
        state = sm.trigger<Triggers::OnToOn>();
        Assert::AreEqual<int>(on.getTypeId(), state->getTypeId());
        expected.push_back("OnState<-OnState");
        expected.push_back("OnState::Do");

        // On <- On, unhandled trigger
        state = sm.trigger<Triggers::On>();
        Assert::AreEqual<int>(on.getTypeId(), state->getTypeId());

        // Off <- On, unhandled trigger
        state = sm.trigger<Triggers::Off>();
        Assert::AreEqual<int>(off.getTypeId(), state->getTypeId());
        expected.push_back("OnState::Exit");
        expected.push_back("OffState<-OnState");
        expected.push_back("OffState::Entry");
        expected.push_back("OffState::Do");

        state = sm.trigger<Triggers::OffToFinal>();
        expected.push_back("OffState::Exit");
        expected.push_back("Final<-OffState");

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
