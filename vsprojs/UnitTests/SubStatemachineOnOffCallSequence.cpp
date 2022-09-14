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

#include <vector>

namespace UnitTests {

  namespace SubstateRecording {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace std;

    typedef State<VirtualGetTypeIdStateComperator, false> StateType;
    typedef FactorCreator<StateType, false> StateTypeCreationPolicyType;

    struct InitialStateFake : StateType {
      static const char* Name;
    };
    const char* InitialStateFake::Name = "Initial";

    //struct AnyStateFake : SimpleState<AnyStateFake, StateType> {
    //  
    //  typedef AnyStateFake CreatorType;
    //  typedef AnyStateFake ObjectType;

    //  static AnyStateFake* create() { return 0; }
    //  static void destroy(AnyStateFake*) { }

    //};
    //

    struct AnyStateFake : SimpleState<AnyStateFake, StateType>, StateTypeCreationPolicyType {
      static const char* Name;

    private:
      friend class SimpleState<AnyStateFake, StateType>;
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
      void entry() { recorder.push_back("IdleState::Entry"); }
      void exit() { recorder.push_back("IdleState::Exit"); }
      template<uint8_t N>
      void doit() { recorder.push_back("IdleState::Do"); }
    };
    const char* IdleState::Name = "IdleState";

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
    typedef ActionSpy<struct IdleState, struct OnState> ToIdleFromOnActionSpy;

    typedef Transition<Triggers::On, OnState, OffState, StateTypeCreationPolicyType, OkGuard, ToOnFromOffActionSpy> ToOnFromOffTransition;
    typedef Transition<Triggers::Off, OffState, OnState, StateTypeCreationPolicyType, OkGuard, ToOffFromOnActionSpy> ToOffFromOnTransition;
    typedef ExitTransition<Triggers::GoodbyeSub, IdleState, OnState, StateTypeCreationPolicyType, OkGuard, ToIdleFromOnActionSpy> ToIdleFromOnTransition;

    typedef
      Typelist<ToOnFromOffTransition,
      Typelist<ToOffFromOnTransition,
      Typelist<ToIdleFromOnTransition,
      NullType>>> ActivestateTransitionList;

    typedef ActionSpy<struct OffState, struct InitialStateFake> ToOffFromInitialActionSpy;
    typedef InitialTransition<OffState, StateTypeCreationPolicyType, ToOffFromInitialActionSpy> ActivestateInitTransition;
    typedef Statemachine<
      ActivestateTransitionList,
      ActivestateInitTransition,
      NullEndTransition<StateTypeCreationPolicyType>> ActivestateStatemachine;

    struct ActiveState : SubstatesHolderState<ActiveState, StateType, ActivestateStatemachine>, FactorCreator<ActiveState> {
      static const char* Name;

      uint8_t getTypeId() const override { return 3; }

    private:
      friend class SubstatesHolderState<ActiveState, StateType, ActivestateStatemachine>;
      void entry() { recorder.push_back("ActiveState::Entry"); }
      void exit() { recorder.push_back("ActiveState::Exit"); }
      template<uint8_t N>
      void doit() { recorder.push_back("ActiveState::Do"); }
    };
    const char* ActiveState::Name = "ActiveState";

    // sub-states transitions are self transitions
    typedef Declaration<Triggers::On, ActiveState, StateTypeCreationPolicyType> ToOnFromOffSubTransition;
    typedef Declaration<Triggers::Off, ActiveState, StateTypeCreationPolicyType> ToOffFromOnSubTransition;
    typedef ExitDeclaration<Triggers::GoodbyeSub, IdleState, ActiveState, StateTypeCreationPolicyType> ToIdleFromOffSubTransition;

    typedef ActionSpy<struct ActiveState, struct IdleState> ToActiveFromIdleActionSpy;
    typedef ActionSpy<struct IdleState, struct AnyStateFake> ToIdleFromAnyActionSpy;

    typedef Transition<Triggers::Hello, ActiveState, IdleState, StateTypeCreationPolicyType, OkGuard, ToActiveFromIdleActionSpy> ToActiveFromIdleTransition;
    //typedef Transition<Triggers::Goodbye, StateType, IdleState, AnyState<StateType>, OkGuard, ToIdleFromAnyActionSpy> ToIdleFromActiveTransition;

    typedef
      Typelist<ToOnFromOffSubTransition,
      Typelist<ToOffFromOnSubTransition,
      Typelist<ToIdleFromOffSubTransition,
      Typelist<ToActiveFromIdleTransition,
      //Typelist<ToIdleFromActiveTransition,
      NullType>>>> TransitionList;

    struct ActiveStateFinalizeGuard {
      template<typename T>
      bool check(T*) { return true; }
    };

    typedef ActionSpy<struct IdleState, struct InitialStateFake> ToIdleFromInitialActionSpy;
    typedef InitialTransition<IdleState, StateTypeCreationPolicyType, ToIdleFromInitialActionSpy> InitTransition;
    typedef Statemachine<
      TransitionList,
      InitTransition,
      NullEndTransition<StateTypeCreationPolicyType>> Sm;

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
