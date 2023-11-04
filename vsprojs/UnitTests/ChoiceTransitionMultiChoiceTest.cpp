/*
  Copyright 2022-2023 Stefan Grimm

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
#include "NotquiteBDD.h"
#include "../../src/tsm.h"
#include "TestHelpers.h"

namespace UT {
  namespace Transitions {

    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace ChoiceTransitionMultiChoiceTestImpl {

      using StatePolicy = State<VirtualTypeIdComparator, false>;
      using RecorderType = Recorder<sizeof(__FILE__) + __LINE__>;
      template<class Derived> struct Leaf : BasicState<Derived, StatePolicy, true, true, true>, FactoryCreatorFake<Derived> {};

      template<class From>
      struct ActionChoiceRecordingSpy {
        template<class StateType, class EventType>
        void perform(StateType&, const EventType&) {
          ostringstream buf;
          buf << "?<-" << From::name;
          RecorderType::add(buf.str());
        }
      };

      namespace Trigger
      {
        struct Choice_A_B_C {};
        struct Choice_A_B_Final {};
      }

      struct A : Leaf<A> {
        static const char* name;

        template<class Event> void entry(const Event&) { RecorderType::add("A::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("A::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("A::Do"); }
        uint8_t getTypeId() const override { return 1; };
      };
      const char* A::name = "A";

      struct B : Leaf<B> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("B::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("B::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("B::Do"); }

        uint8_t getTypeId() const override { return 2; };
      };
      const char* B::name = "B";

      struct C : Leaf<C> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("C::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("C::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("C::Do"); }

        uint8_t getTypeId() const override { return 3; };
      };
      const char* C::name = "C";

      struct Choice1GuardStub {
        static bool ReturnValue;
        template<class StateType, class EventType>
        bool eval(const StateType&, const EventType&) {
          return ReturnValue;
        }
      };
      bool Choice1GuardStub::ReturnValue = false;

      struct Choice2GuardStub {
        static bool ReturnValue;
        template<class StateType, class EventType>
        bool eval(const StateType&, const EventType&) {
          return ReturnValue;
        }
      };
      bool Choice2GuardStub::ReturnValue = false;

      using ToAorBorC = Choice2Transition<Trigger::Choice_A_B_C, B, C, A, A, Choice1GuardStub, Choice2GuardStub, ActionChoiceRecordingSpy<A>>;
      using ToAorBorExit = Choice2Transition<Trigger::Choice_A_B_Final, A, B, EmptyState<StatePolicy>, A, Choice1GuardStub, Choice2GuardStub, ActionChoiceRecordingSpy<A>>;
      using Transitions =
        Typelist<ToAorBorC,
        Typelist<ToAorBorExit,
        NullType>>;

      using ToplevelInitTransition = InitialTransition<A, ActionSpy<A, InitialStateNamedFake<StatePolicy>, RecorderType>>;
      using Sm = Statemachine<Transitions, ToplevelInitTransition>;
    }

    BEGIN(
      ChoiceTransitionMultiChoiceTest,
      "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=ChoiceTransitionMultiChoiceTest.drawio#R7Vptc6I6FP41ztz90A7v6ke12n27O52xs7u9XzoRInA3EBtilf76m5SghCCihbp3ZtsZTQ4hwDnPeXkO9sxJtL0lYBX8jT2IeobmbXvmTc8who7DPrkgzQTO0M4EPgm9TKTvBfPwBQqhJqTr0IOJtJBijGi4koUujmPoUkkGCMEbedkSI%2FmqK%2BBDRTB3AVKlP0KPBpl0YGt7%2BUcY%2BkF%2BZV0TRyKQLxaCJAAe3hRE5rRnTgjGNBtF2wlEXHe5XrLzZgeO7m6MwJg2OYFu7u4hSul09Gmzjacvd8%2FTwZXY5RmgtXjgJBK3S9NcB%2BzOV3y4jtCMgIgNx5sgpHC%2BAi6Xb5jlmSygEWIznQ0JXsce9MRs9%2BQam7g4Cl0xRmAB0Ri4v%2FzXEyYYYcIOxTjm10gowb92WucbLXFMZyAKEQfTd0g8EAMhFsjRmVrHAIV%2BzCYu0wxkG45VVeXPDQmF24JIqO4W4ghSkrIl4qgprJjK080eE8McE0EBD7YlhEDg0N%2FtvDcVGwhrnWA5Q7HcSDGcbIYjNgPEFUo0LK7UEKHcID3DXLI%2FV9tZRTqisb%2FzzdzQqHxeuKqmiasqxuY2DZkDj8QBilftQMCwZAwMVAzslhQxYGpdYcBUMDD%2Bg4FOMeAMzsOA0RkGBorFoccymJhiQgPs4xig6V5aCM9ce%2Fs1XzHX0ysS%2FoWUpkL3YE2xjJODmkzwmriw5naHIocD4kN6PLzxZ6m1C4EI0PBZztatK3moOJqqdYRYTQJlPeWZM2HPS%2BcUUKh61Q7Fh72qlXTato95IWEFV4i5kJmdw78ND9Md2cMs1cPMyiD7dtMP0Ffru5Ui13KDT6un1P6GF1d9xfSTPzH2XWOsabxnkK0EQc5GiigIcOjCx9Hj%2BLEOEGcGWKY9kv4U579OHvjk2s6nN9viwZs0n21D%2BrMwzs4ybDHdn8Un%2BUnnR3NDjebV%2BtMbhnMBgSvtWrP6jgSD3AaNI77Y%2FQ6H7LEOVnGmZstbZA8tziryqXyjfCFeLhNIFWztnuYNcFNpmQqxAEeLdXI83rTgkaZly6a4dFDWVfbTs8f%2BGhCPSfVXlNA1iRMOTsKW2Dcd%2BGjua9d20dtqXW3v19eabsq%2BzS1Q79x8dgdJyJTIQ3AmPFrtNXLvWhwerdbMk93bHjr9Tty7L7u3YZ7q3mIjq1SHMC4vb5Qp5bJxQiWAzA2WACWViIexN%2BItsX1NUNmyKeHULqJUb5p%2FTvCJN%2BDTajv%2FFAJaZTunHZCaZWyVA2VjkOoHSqUjIG0NgNbxRHWsHE5WWd92GW45%2FFrJV7bdqwwCBfPqFebVW8hXIewT4%2BHpYT7%2FMbZfPn5xoy83FW3WBb%2FdOYVLELPBLYvpUY8zC%2B2vyQf2yf7NbI5YlRknkOc17qCEwz2A7HPCj44fruYj9m1da%2FzckNfmMXcFLgwo5V3yEb93Y%2BYS%2BIpZxhgiHCfXmPhMKrZniXK2SK8SwL5f95p9UAzJlE9L9FbiKDnPKBAaIcq5A4JLWsEcotDz0KGushyXCmxl2Elj167o6NhaJdnoCCtqaXNPQt%2BHJDOlg7gKFwwHjs9HMg%2BpOToLWV3wfkZdYEpx1MioLVixVNRbtmpFvcqKehsu%2F80dozQYPtz%2FY33G2F2%2B%2BM%2BowuUV3V%2ByhDeG71fCV%2BpHhXkBq1NWH6jq%2Br1Idf8dSXUdwn4HTm3L7qc77XFqtXgqJXflYu3VPJVqr6i5lUK7rhfMqvDzO8EdNV3L0cHqXzg6VNSVdcSm%2Bzceb44dEiHS3x466kLC0dBxOl%2FvJG6UeZBVzsZNeVCpY2wNmtGgjrh6pcrtKkSf1rE6h7%2FrbaS6FjJdpU6czjKdYzqWBIkrXaWAHTJ8a1hi%2BP0z21BmaSOlXX0A2gwoIC0sW%2FEFyaXA7xzPmRdoE1jKj3qqWIOKkTY4w4u5fHwcJI9P8zgG889oGjvzKs7AYwTJgoTRKEi01tbWTwgBh7KjcTTeNOxel1y%2FvXb24Xe%2BdeGlL5dLeXB%2Fazos9a51%2B8x06AxNeSPTumQ%2BrMS6yv8K%2BfCdoX7eGxwZ6v1LQb1pIXj4xc0l3tP8L7B%2BOIk27Ov3tdp7t8svq05db0rr2SC744aOyqb7nx9ny%2Fe%2F4Tan%2FwE%3D")

      INIT(
        Initialize,
        {
          using namespace ChoiceTransitionMultiChoiceTestImpl;
          RecorderType::reset();
          FactoryCreatorFake<A>::reset();
          FactoryCreatorFake<B>::reset();
          FactoryCreatorFake<C>::reset();
          Choice1GuardStub::ReturnValue = false;
          Choice2GuardStub::ReturnValue = false;
        })

      TEST(
        ChoiceAorBorC,
        WhenChoice1ReturnsTrueandChoice2ReturnsFalse,
        ActiveStateB)
      {
        using namespace ChoiceTransitionMultiChoiceTestImpl;
        Sm sm;
        Choice1GuardStub::ReturnValue = true;
        Choice2GuardStub::ReturnValue = false;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_A_B_C>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        NEQ(0, FactoryCreatorFake<B>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST(
        ChoiceAorBorC,
        WhenChoice2ReturnsTrueandChoice1ReturnsFalse,
        ActiveStateC)
      {
        using namespace ChoiceTransitionMultiChoiceTestImpl;
        Sm sm;
        Choice1GuardStub::ReturnValue = false;
        Choice2GuardStub::ReturnValue = true;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_A_B_C>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "C::Entry",
          "C::Do" });

        // Active state is C
        NEQ(0, FactoryCreatorFake<C>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls + 1);
      }

      TEST(
        ChoiceAorBorC,
        WhenChoice1andChoice2ReturnFalse,
        ActiveStateA)
      {
        using namespace ChoiceTransitionMultiChoiceTestImpl;
        Sm sm;
        Choice1GuardStub::ReturnValue = false;
        Choice2GuardStub::ReturnValue = false;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_A_B_C>();
        RecorderType::check({
          "?<-A",
          "A::Do" });

        // Active state is A
        NEQ(0, FactoryCreatorFake<A>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls + 1);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST(
        ChoiceAorBorExit,
        WhenChoice1ReturnsTrueandChoice2ReturnsFalse,
        ActiveStateA)
      {
        using namespace ChoiceTransitionMultiChoiceTestImpl;
        Sm sm;
        Choice1GuardStub::ReturnValue = true;
        Choice2GuardStub::ReturnValue = false;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_A_B_Final>();
        RecorderType::check({
          "?<-A",
          "A::Do" });

        // Active state is A
        NEQ(0, FactoryCreatorFake<A>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls + 1);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST(
        ChoiceAorBorExit,
        WhenChoice2ReturnsTrueandChoice1ReturnsFalse,
        ActiveStateB)
      {
        using namespace ChoiceTransitionMultiChoiceTestImpl;
        Sm sm;
        Choice1GuardStub::ReturnValue = false;
        Choice2GuardStub::ReturnValue = true;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_A_B_Final>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        NEQ(0, FactoryCreatorFake<B>::createCalls);
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST(
        ChoiceAorBorExit,
        WhenChoice1andChoice2ReturnFalse,
        ThenFinalState)
      {
        using namespace ChoiceTransitionMultiChoiceTestImpl;
        Sm sm;
        Choice1GuardStub::ReturnValue = false;
        Choice2GuardStub::ReturnValue = false;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_A_B_Final>();
        RecorderType::check({
          "?<-A",
          "A::Exit" });

        // States A, B and C are deactivated
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

    END

  }
}
