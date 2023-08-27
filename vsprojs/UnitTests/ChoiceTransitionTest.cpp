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
#include "../../src/tsm.h"
#include "TestHelpers.h"

namespace UT {
  namespace Transitions {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace ChoiceTransitionTestImpl {

      using StateType = State<VirtualGetTypeIdStateComparator, false>;
      using StateTypeCreationPolicyType = FactoryCreatorFake<StateType>;
      using RecorderType = Recorder<sizeof(__FILE__) + __LINE__>;
      template<class Derived> struct Leaf : BasicState<Derived, StateType, true, true, true>, FactoryCreatorFake<Derived> {};

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
        struct Choice_B_C {};
        struct Choice_A_B {};
        struct Count {};
      }

      struct A : Leaf<A> {
        static const char* name;
        int counter = 0;

        template<class Event> void entry(const Event&) { RecorderType::add("A::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("A::Exit"); }
        template<class Event> void doit(const Event&) {
          RecorderType::add("A::Do");
          if (is_same<Event, Trigger::Count>().value) {
            counter++;
          }
        }
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

      struct ChoiceGuardStub {
        static bool ReturnValue;
        template<class StateType, class EventType>
        bool eval(const StateType&, const EventType&) {
          return ReturnValue;
        }
      };
      bool ChoiceGuardStub::ReturnValue = true;

      struct CountTriggerGuardA {
        template<class StateType, class EventType>
        bool eval(const StateType& state, const EventType& ) {
          return state.counter < 3;
        }
      };

      using ToAorB = ChoiceTransition<Trigger::Choice_A_B, B, A, A, StateTypeCreationPolicyType, ChoiceGuardStub, ActionChoiceRecordingSpy<A>>;
      using ToBorC = ChoiceTransition<Trigger::Choice_B_C, B, C, A, StateTypeCreationPolicyType, ChoiceGuardStub, ActionChoiceRecordingSpy<A>>;
      using ToAorFinal = ChoiceTransition<Trigger::Count, A, EmptyState<StateType>, A, StateTypeCreationPolicyType, CountTriggerGuardA, ActionChoiceRecordingSpy<A>>;
      using Transitions = 
        Typelist<ToAorB,
        Typelist<ToBorC,
        Typelist<ToAorFinal,
        NullType>>>;

      using ToplevelInitTransition = InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateNamedFake<StateType>, RecorderType>>;
      using Sm = Statemachine<Transitions, ToplevelInitTransition>;
    }

    // https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=ChoiceTransitionTest.drawio#R7Vtbc9o4FP41zGwfkvGVyyOQS7vtdjJDpm32JSNsYXsrW1QWAfLr9whksC1hA7VJHprMBJ2ji61zvnOTSMcex6t7hubhP9THpGMZ%2Fqpj33QsyzQMAz4EZ73ldAf9LSNgkS8H7RmT6BVnMyV3Efk4LQzklBIezYtMjyYJ9niBhxijy%2BKwGSXFp85RgBXGxENE5X6PfB5uuX3X2PM%2F4igIsyeb2YZjlA2WjDREPl3mWPZtxx4zSvm2Fa%2FGmAjhZXLZzrs70Lt7MYYTfswEvnx4xGTNb4eflqvk9vXh5bZ%2FJVd5QWQhN5zG8nX5OpMBvPlcNBcxuWMohuZoGUYcT%2BbIE%2FwlqB54IY8JUCY0GV0kPvYltdu5AYRH48iTbYKmmIyQ9zPYTBhTQhl0JTQRz0g5oz93UhcLzWjC71AcEQGmb5j5KEGSLZFjglhHiERBAoQHksGw4EgVVbZvzDhe5VhSdPeYxpizNQyRvbbU4rpILveYGGSYCHN4cB3JRBKHwW7lvaqgIbV1guYsRXNDRXFFNdToDDFPCtFyhFAjQjKFdCx7Bj%2BesdNKoccQVn62mo9UqqBzTzUM%2BVRF2UKnERjwUHZwOm8GApZTxEBfxcBuSB4DttEWBmwFA6M%2FGGgVA93%2BeRiwWsNAX9E49iGCSZIyHtKAJojc7rk59yyktx%2FzhQo5bZDwH%2BZ8LWWPFpwWcXJQkildMA9XvO5AxnDEAszr3ZvYS6VeGCaIRy%2FFaN24kAeKoalSJwRyElyUUxY5U9gvn3DEsWpVOxQftqpGwmnTNuZHDBKuiAomqF3AvwkLM7tFC3NUC7O1TrYl1ZuOovtxSCMPP4%2BexxXe9kzLAsGx9Q85f0M8CeLazcibVb7zZp1Rq4j%2FyNaAdm4WUPtJgsjmnG%2FFlmrFffLF%2Beasied44af5r7X7lU6PtmKp%2Byvj2oSfgv6zQuBoQ5erP9AIdrUfQmezFN61DIfdSxyFkKpNVjkHFtJ4ukjrA3ADBuT2igZkme7FLEgrH1eVjzviDAj3pgUDOsMQDhldr87qjrKgKtDUxkH7ZAtyGzUfua4zKILKHJTQst2wnJWvQksLuW7NQluJVCzUvkH3VI9fgdM%2FaXXzabVtXTKv1oKgr%2FNaM0TSdt3W3lU95XpOc1u1uUJtgdCoX6uysRP82qBrFz2H24hn6xoNebZunYt8B54tiwZqMjt8rjo6eKNk1ujkY7j11tlslpmeAFrD6XXbiMflsyjbcE9E7QXAph4uqxC7YGJsO6XU6IKlpV5Amsrh3WbG14ZpF61YyLrajAX1gFkE4hL5Q%2BvBJ0NcC1n1wDDbMeRytWafashZhl46N4FU%2B%2F2FH%2FXAuiqpwok%2FFFd4%2B4RWe8VUwqmbR6l5bKQppF3G7wca%2Ff6dpiNNznVpr5%2BaAaldxlbZJR4NUvNAnl8D0sYAqB7mKairq%2BXS%2BfaeeRatBPwaiUyu29E6gZx6TY16zQYiU4R7zHr69TSZfB%2B5rx8%2Fe%2FHnG8218FS87oTjGUqgcQ8%2BPe6Istj4a%2FwB%2FsKvvaUJZJNJCpKxDGGgTMA9xPB3LHpHT1eTIXw614aYG4nCMhGmIJgh5%2BJWfyje3brzGN5gFsrdmCbpNWUBcOXyKTSn66sUwedmrbsPiiJB%2BLx0HF8osLMiOVeNS1ZW%2BBI845qyN458fxOpdFAp%2BqVcqT1oCCtFC3I1N1Cuoa2UW8KKmsQ8sigIMNuqskuECKeAg24gWvl640Df5mBd6QO58svpeEo5p%2FFROm5AqaVs3nFVpZo6pZpNeICv3oisw8HT47%2FO35R6s9fghWg8gCL7t8zdrcHlcnetfFTU6wH6zmro3gVr6CpcvYcSunwS3W2uhFYzqFKEVx7WXOKjFbsm8Vay7aoLbEjFz7%2B%2BbummuOwTnN4b%2BwRNcumOPOEWNmkQRDIUC9e4iWfAsLVFT%2Fvf3vhtl3LCGfVRHqXKU9R6lNNr%2BaI7sdupkZxyaD62RipdhTj940qklup4rci1N7oaoG9SuM2AA1g%2Fo8A3z8TsCYde52O221oUVL4XcaW51m%2FxCKB8AWz1zjynsksLKSfXB%2FANQEHr3LC5GJC%2BlQV06%2BPpG5wjOMq3lHV1hIqRM6oIIPdfXd9Kdf8PAPbt%2Fw%3D%3D
    TEST_CLASS(ChoiceTransitionTest)
    {
      TEST_METHOD_INITIALIZE(Initialize)
      {
        using namespace ChoiceTransitionTestImpl;
        RecorderType::reset();
        FactoryCreatorFake<A>::reset();
        FactoryCreatorFake<B>::reset();
        FactoryCreatorFake<C>::reset();
      }

      TEST_METHOD(ChoiceAorB_WhenChoiceReturnsTrue_ThenActiveStateA)
      {
        using namespace ChoiceTransitionTestImpl;
        Sm sm;
        ChoiceGuardStub::ReturnValue = true;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_B_C>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        Assert::AreNotEqual<int>(0, FactoryCreatorFake<B>::createCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
        Assert::AreEqual<int>(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST_METHOD(ChoiceAorB_WhenChoiceReturnsFalse_ThenActiveStateA)
      {
        using namespace ChoiceTransitionTestImpl;
        Sm sm;
        ChoiceGuardStub::ReturnValue = false;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_A_B>();
        RecorderType::check({
          "?<-A",
          "A::Do" });

        // Active state is A
        Assert::AreNotEqual<int>(0, FactoryCreatorFake<A>::createCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls + 1);
        Assert::AreEqual<int>(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST_METHOD(ChoiceBorC_WhenChoiceReturnsTrue_ThenActiveStateB)
      {
        using namespace ChoiceTransitionTestImpl;
        Sm sm;
        ChoiceGuardStub::ReturnValue = true;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_B_C>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        Assert::AreNotEqual<int>(0, FactoryCreatorFake<B>::createCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
        Assert::AreEqual<int>(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }

      TEST_METHOD(ChoiceBorC_WhenChoiceReturnsFalse_ThenActiveStateC)
      {
        using namespace ChoiceTransitionTestImpl;
        Sm sm;
        ChoiceGuardStub::ReturnValue = false;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Choice_B_C>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "C::Entry",
          "C::Do" });

        // Active state is C
        Assert::AreNotEqual<int>(0, FactoryCreatorFake<C>::createCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls + 1);
      }

      TEST_METHOD(Count_WhenCounterOverThreshold_ThenFinalState)
      {
        using namespace ChoiceTransitionTestImpl;
        Sm sm;

        sm.begin();

        sm.dispatch<Trigger::Count>();
        sm.dispatch<Trigger::Count>();
        sm.dispatch<Trigger::Count>();

        RecorderType::reset();

        sm.dispatch<Trigger::Count>();

        RecorderType::check({
          "?<-A",
          "A::Exit" });

        // States A, B and C are deactivated
        Assert::AreEqual<int>(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<C>::createCalls, FactoryCreatorFake<C>::deleteCalls);
      }
    };
  }
}
