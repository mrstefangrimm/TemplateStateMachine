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

namespace UnitTests {

  namespace SubstateRecording {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace std;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;

    struct InitialStateFake : StateType {
      static const char* Name;
    };
    const char* InitialStateFake::Name = "Initial";

    struct AnyStateFake : SimpleState<AnyStateFake, StateType> {
      static const char* Name;
      typedef AnyStateFake CreatorType;
      static AnyStateFake* create() { return 0; }
      static void destroy(AnyStateFake*) { }

    private:
      friend class SimpleState<AnyStateFake, StateType>;
      void entry_() { }
      void exit_() { }
      template<uint8_t N>
      void doit_() { }
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
      On,
      Off,
      Goodbye,
      GoodbyeSub,
      Hello,
    };

    vector<string> recorder;

    struct IdleState : SimpleState<IdleState, StateType>, FactorCreator<IdleState> {
      static const char* Name;

      uint8_t getTypeId() const override { return 1; }

    private:
      friend class SimpleState<IdleState, StateType>;
      void entry_() { recorder.push_back("IdleState::Entry"); }
      void exit_() { recorder.push_back("IdleState::Exit"); }
      template<uint8_t N>
      void doit_() { recorder.push_back("IdleState::Do"); }
    };
    const char* IdleState::Name = "IdleState";

    struct OnState : SimpleState<OnState, StateType>, FactorCreator<OnState> {
      static const char* Name;

      uint8_t getTypeId() const override { return 1; }

    private:
      friend class SimpleState<OnState, StateType>;
      void entry_() { recorder.push_back("OnState::Entry"); }
      void exit_() { recorder.push_back("OnState::Exit"); }
      template<uint8_t N>
      void doit_() { recorder.push_back("OnState::Do"); }
    };
    const char* OnState::Name = "OnState";

    struct OffState : SimpleState<OffState, StateType>, FactorCreator<OffState> {
      static const char* Name;

      uint8_t getTypeId() const override { return 2; }

    private:
      friend class SimpleState<OffState, StateType>;
      void entry_() { recorder.push_back("OffState::Entry"); }
      void exit_() { recorder.push_back("OffState::Exit"); }
      template<uint8_t N>
      void doit_() { recorder.push_back("OffState::Do"); }
    };
    const char* OffState::Name = "OffState";

    typedef ActionSpy<struct OnState, struct OffState> ToOnFromOffActionSpy;
    typedef ActionSpy<struct OffState, struct OnState> ToOffFromOnActionSpy;
    typedef ActionSpy<struct IdleState, struct OnState> ToIdleFromOnActionSpy;

    typedef Transition<Triggers::On, StateType, OnState, OffState, OkGuard, ToOnFromOffActionSpy> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, StateType, OffState, OnState, OkGuard, ToOffFromOnActionSpy> ToOffFromOnTransition;
    typedef ExitTransition<Triggers::GoodbyeSub, StateType, IdleState, OnState, OkGuard, ToIdleFromOnActionSpy> ToIdleFromOnTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToIdleFromOnTransition,
      NullType>>> ActivestateTransitionList;

    typedef ActionSpy<struct OffState, struct InitialStateFake> ToOffFromInitialActionSpy;
    typedef InitialTransition<StateType, OffState, ToOffFromInitialActionSpy> ActivestateInitTransition;
    typedef Statemachine<
      StateType,
      ActivestateTransitionList,
      ActivestateInitTransition,
      NullEndTransition<StateType>> ActivestateStatemachine;

    struct ActiveState : SubstatesHolderState<ActiveState, StateType, ActivestateStatemachine>, FactorCreator<ActiveState> {
      static const char* Name;

      uint8_t getTypeId() const override { return 3; }

    private:
      friend class SubstatesHolderState<ActiveState, StateType, ActivestateStatemachine>;
      void entry_() { recorder.push_back("ActiveState::Entry"); }
      void exit_() { recorder.push_back("ActiveState::Exit"); }
      template<uint8_t N>
      void doit_() { recorder.push_back("ActiveState::Do"); }
    };
    const char* ActiveState::Name = "ActiveState";

    // sub-states transitions are self transitions
    typedef Declaration<Triggers::On, StateType, ActiveState> ToOnFromOffSubTransition;
    typedef Declaration<Triggers::Off, StateType, ActiveState> ToOffFromOnSubTransition;
    typedef Declaration<Triggers::GoodbyeSub, StateType, ActiveState> ToIdleFromOffSubTransition;

    typedef ActionSpy<struct ActiveState, struct IdleState> ToActiveFromIdleActionSpy;
    typedef ActionSpy<struct IdleState, struct AnyStateFake> ToIdleFromAnyActionSpy;

    typedef Transition<Triggers::Hello, StateType, ActiveState, IdleState, OkGuard, ToActiveFromIdleActionSpy> ToActiveFromIdleTransition;
    typedef Transition<Triggers::Goodbye, StateType, IdleState, AnyState<StateType>, OkGuard, ToIdleFromAnyActionSpy> ToIdleFromActiveTransition;

    typedef
      Typelist<ToOnFromOffSubTransition,
      Typelist<ToOffFromOnSubTransition,
      Typelist<ToIdleFromOffSubTransition,
      Typelist<ToActiveFromIdleTransition,
      Typelist<ToIdleFromActiveTransition,
      NullType>>>>> TransitionList;

    struct ActiveStateFinalizeGuard {
      template<typename T>
      bool check(T*) { return true; }
    };

    typedef ActionSpy<struct IdleState, struct InitialStateFake> ToIdleFromInitialActionSpy;
    typedef InitialTransition<StateType, IdleState, ToIdleFromInitialActionSpy> InitTransition;
    //typedef FinalTransition<StateType, ActiveStateFinalizeGuard, EmptyAction> ActivestateFinalTransition;
    typedef Statemachine<
      StateType,
      TransitionList,
      InitTransition,
      NullEndTransition<StateType>> Sm;

    TEST_CLASS(SubstatemachineOnOffInitialAndFinalTransitions)
    {
    public:

      TEST_METHOD(ExitfromActiveExitsSubstate)
      {
        //  +[0]	"IdleState<-Initial"	std::string
        //  + [1]	"IdleState::Entry"	std::string
        //  + [2]	"IdleState::Do"	std::string
        //  + [3]	"IdleState::Exit"	std::string
        //  + [4]	"ActiveState<-IdleState"	std::string
        //  + [5]	"ActiveState::Entry"	std::string
        //  + [6]	"OffState<-Initial"	std::string
        //  + [7]	"OffState::Entry"	std::string
        //  + [8]	"OffState::Do"	std::string
        //  + [9]	"ActiveState::Do"	std::string
        //  + [10]	"OffState::Exit"	std::string
        //  + [11]	"OnState<-OffState"	std::string
        //  + [12]	"OnState::Entry"	std::string
        //  + [13]	"OnState::Do"	std::string
        // TODO: IdleState<-AnyStateFake should be here, or not (QM/QP does it like that)
        //  + [14]	"OnState::Exit"	std::string
        //  + [15]	"ActiveState::Exit"	std::string
        //  + [16]	"IdleState<-AnyStateFake"	std::string
        //  TODO: idle::enter idle::do are missing

        Sm sm;

        sm.begin();

        sm.dispatch<Triggers::Hello>();

        sm.dispatch<Triggers::On>();

        sm.dispatch<Triggers::Goodbye>();
      }

      TEST_METHOD(transitionfromsubstateout_subandsupperstateexitiscalled)
      {
        //   +[0]	"IdleState<-Initial"	std::string
        //  + [1]	"IdleState::Entry"	std::string
        //  + [2]	"IdleState::Do"	std::string
        //  + [3]	"IdleState::Exit"	std::string
        //  + [4]	"ActiveState<-IdleState"	std::string
        //  + [5]	"ActiveState::Entry"	std::string
        //  + [6]	"OffState<-Initial"	std::string
        //  + [7]	"OffState::Entry"	std::string
        //  + [8]	"OffState::Do"	std::string
        //  + [9]	"ActiveState::Do"	std::string
        //  + [10]	"OffState::Exit"	std::string
        //  + [11]	"OnState<-OffState"	std::string
        //  + [12]	"OnState::Entry"	std::string
        //  + [13]	"OnState::Do"	std::string
        // 
        //  + [14]	"OnState::Exit"	std::string
        //  + [15]	"ActiveState::Exit"	std::string
        //  TODO: Exit okay so far, but transition to idle and idle::enter idle::do are missing

        Sm sm;

        sm.begin();

        sm.dispatch<Triggers::Hello>();

        sm.dispatch<Triggers::On>();

        sm.dispatch<Triggers::GoodbyeSub>();
      }

    };
  }
}
