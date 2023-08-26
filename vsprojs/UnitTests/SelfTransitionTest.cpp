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
#define IAMWORKSTATION 1

#include "CppUnitTest.h"

#include "../../src/tsm.h"
#include "TestHelpers.h"

namespace UT {
  namespace Transitions {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace SelfTransitionTestImpl {

      typedef State<MemoryAddressComparator, true> StateType;
      typedef SingletonCreatorFake<StateType> StateTypeCreationPolicyType;
      typedef Recorder<sizeof(__FILE__) + __LINE__> RecorderType;
      template<class Derived> struct Leaf : BasicState<Derived, StateType>, SingletonCreatorFake<Derived> {};

      struct InitialStateFake : StateType {
        static const char* name;
      };
      const char* InitialStateFake::name = "Initial";

      template<class T>
      struct EmptyStateFake : T {
        static const char* name;

        typedef EmptyStateFake CreatorType;
        typedef EmptyStateFake ObjectType;

        static EmptyStateFake* create() { return nullptr; }
        static void destroy(EmptyStateFake*) { }

        template<class Event> void _entry(const Event&) {}
        template<class Event> bool _doit(const Event& ev) { return false; }
      };
      template<class T> const char* EmptyStateFake<T>::name = "Final";

      namespace Trigger
      {
        struct Next {};
        struct Self {};
        struct Reenter {};
      }

      struct A : Leaf<A> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("A::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("A::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("A::Do"); }
      };
      const char* A::name = "A";

      struct B : Leaf<B> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("B::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("B::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("B::Do"); }
      };
      const char* B::name = "B";

      struct C : Leaf<C> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("C::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("C::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("C::Do"); }
      };
      const char* C::name = "C";

      typedef Transition<Trigger::Next, B, A, StateTypeCreationPolicyType, NoGuard, ActionSpy<B, A, RecorderType>> B_A_t;
      typedef Transition<Trigger::Next, C, B, StateTypeCreationPolicyType, NoGuard, ActionSpy<C, B, RecorderType>> C_B_t;
      typedef SelfTransition<Trigger::Reenter, A, StateTypeCreationPolicyType, NoGuard, ActionSpy<A, A, RecorderType>, true> A_A_rt;
      typedef SelfTransition<Trigger::Reenter, B, StateTypeCreationPolicyType, NoGuard, ActionSpy<B, B, RecorderType>, true> B_B_rt;
      typedef SelfTransition<Trigger::Reenter, C, StateTypeCreationPolicyType, NoGuard, ActionSpy<C, C, RecorderType>, true> C_C_rt;
      typedef SelfTransition<Trigger::Self, A, StateTypeCreationPolicyType, NoGuard, ActionSpy<A, A, RecorderType>, false> A_A_t;
      typedef SelfTransition<Trigger::Self, B, StateTypeCreationPolicyType, NoGuard, ActionSpy<B, B, RecorderType>, false> B_B_t;
      typedef SelfTransition<Trigger::Self, C, StateTypeCreationPolicyType, NoGuard, ActionSpy<C, C, RecorderType>, false> C_C_t;

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

      typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake, RecorderType>> ToplevelInitTransition;
      typedef Statemachine<
        Transitions,
        ToplevelInitTransition> Sm;
    }

    // https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=SelfTransitionTest.drawio#R7Vpbc9o4FP41zGwfyPiCwTwCgbQzbSdbsmmyLzvCFrY2skVlEXB%2B%2FUpYwheZS4hzaTZkJkjHkmyd851Pn4Rb9ihaX1CwCL8RH%2BKWZfjrln3esizTMAz%2BJSxpZun23cwQUOTLRrlhih6g6imtS%2BTDpNSQEYIZWpSNHolj6LGSDVBKVuVmc4LLd12AAGqGqQewbv2JfBZmVtcxcvtniIJQ3dlUE46AaiwNSQh8siqY7HHLHlFCWFaK1iOIhfOUX7J%2Bkx1Xtw9GYcyO6cBWl1cQp2w8%2BLJax%2BOHy%2Fux25aj3AO8lBNOIvm4LFU%2B4E%2B%2BEMVlhCcURLw4XIWIwekCeMK%2B4qHntpBFmNdMXqRkGfvQl7XtzA1e8UiEPFnGYAbxEHh3wabDiGBC%2BaWYxOIeCaPkbut1MdCcxGwCIoQFmK4h9UEMpFkix%2BRuHQKMgphXPO4ZyAcc6q5S84aUwXXBJF13AUkEGU15E3nVllFMy9VVARMKsmEBD7ZqCCQOg%2B3Ieah4QUarPnL%2FUDp0b3%2FcGOf0b2v82br7NYNt09VC9x2umRY86HM8yyqhLCQBiQEe59ZCsERM8jZfCVlIx%2F8LGUuli8GSkXK0uVdpeiP7byq3onLWc1T9fF28ep6q2hqxGzUILxe78WreS1RUp53BTMiSenAP2C3JH4AGkO1xbT9rJzy3FxoSC6aTtacQA4buy8RRF3c50iVBfAZ5EzKfJ5BpwNje8Cis7Jt4ASoDDSfllD2Q34B6Eg1WRyQgwlglb8uy5%2FzjGdsMLl0xxIpwMiUcSQCiXrirYci7asQg8h9xsh%2FIC0wgvgm6MHtlvnB1vtg2KfLFdrV4Cl%2FUYsB9XWY4c0rccIAYTs%2Fxvp7j%2B1LiYI4%2FLqkHlIK00GAhUjypyXkJE8txSjCR1TzO2YCNkkFfIwMdGRhzfQXLsVQqIOG%2BZVMGGNQzf5tpuzO%2FEWnQNA%2F4iHLxiIgwcoiJFG2EBbplFujoLGDXiYYGOMDFXzvXnRR7HS%2F8sviVOt%2FJrG3aWuynEM%2F1%2BMf%2BQGjn3OG12q64hKvybZ7sO1fwXXrBOqQXjqKF%2Bpl3GuYFGdG2WvwLIXVqQqpsp2oEpUIrgNKgkjlC9ipuBioDdcyKnq2uO5mjtIF2E5x%2Bq0Z1Ta1QczQ4%2F4BZSr%2FGWlcrZq0jU8Esp0IjmXCsCH5SIkhAtfkz99x%2BCVTtpypjRaNO%2FbCNAmwvbexbLA9J5WSRnUvM0VrAq4l1xaqoS7NmO1q3GzWfbTPa0%2Fz0apvRU%2FaUJ%2B9gj8rDfZvMg5tRtWb%2FlrvRfTMvYGWoAeVjN9pqcDdqu1aJL150N1qPakMDwftTovUzb%2FoUKl%2BATf6pXylfRp06TanT7rtQp6Z%2BuP4hT5taFh%2BfHV2rkh1vf6VUXPHG5GfnzclP%2FVxj9CEpnlVSOP3OW5MU%2Bl7t%2FyIpukeS5mP3Ei%2BsH3puQ%2FrBrf5a%2B3vqh4%2FjrRNywW46F97LAdde6nhjCqPbfz2F8ec3ckfTydXw4a9p6Ex%2B0vU1qnlPZiYed8rgHMS8cEFRFLWE6jD%2BGH3i%2F%2FmfndUx4qtxwj1jGSIFqQBnCPn%2Fkbg6vG1PB%2Fy7c2aIvkikdyyAK4whY%2BI1p4F4dmviUbgBGVcTEYmTM0IDbpXDc5aazNJ2Avj3ZqzJJy2QTBzJlX%2FTK%2BkXpUEKYkealK7AcM5qVEWEfH%2FDMnVQKTNPQcn0GzrcqHB9T4eKY9QpkefCii7arygKAkizUHaxcOGM46AbiFJ2UFq1ZtKlat3F%2F88W2RlhjERHRbaBUHYq63%2FN76VmXSjNE%2FKeV%2FN37zI6zd9gtMf%2FAQ%3D%3D
    TEST_CLASS(SelfTransitionTest)
    {
    public:
      TEST_METHOD_INITIALIZE(Initialize)
      {
        using namespace SelfTransitionTestImpl;
        RecorderType::reset();
        SingletonCreatorFake<A>::reset();
        SingletonCreatorFake<B>::reset();
        SingletonCreatorFake<C>::reset();
      }

      TEST_METHOD(Self_StateA)
      {
        using namespace SelfTransitionTestImpl;
        Sm sm;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Self>();
        RecorderType::check({
          "A<-A",
          "A::Do" });

        sm.dispatch<Trigger::Reenter>();
        RecorderType::check({
          "A<-A",
          "A::Exit",
          "A::Entry",
          "A::Do" });

        // Active state is A
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
      }

      TEST_METHOD(Self_StateB)
      {
        using namespace SelfTransitionTestImpl;
        Sm sm;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Next>();
        RecorderType::check({
          "B<-A",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        sm.dispatch<Trigger::Self>();
        RecorderType::check({
          "B<-B",
          "B::Do" });

        sm.dispatch<Trigger::Reenter>();
        RecorderType::check({
          "B<-B",
          "B::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
      }

      TEST_METHOD(Self_StateC)
      {
        using namespace SelfTransitionTestImpl;
        Sm sm;

        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.dispatch<Trigger::Next>();
        RecorderType::check({
          "B<-A",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        sm.dispatch<Trigger::Next>();
        RecorderType::check({
          "C<-B",
          "B::Exit",
          "C::Entry",
          "C::Do" });

        sm.dispatch<Trigger::Self>();
        RecorderType::check({
          "C<-C",
          "C::Do" });

        sm.dispatch<Trigger::Reenter>();
        RecorderType::check({
          "C<-C",
          "C::Exit",
          "C::Entry" ,
          "C::Do" });

        // Active state is C
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls + 1);
      }
    };
  }
}
