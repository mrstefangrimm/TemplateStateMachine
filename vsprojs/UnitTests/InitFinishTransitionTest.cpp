/*
  Copyright 2023 Stefan Grimm

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

    namespace InitFinishTransitionTestImpl {

      using StatePolicy = State<MemoryAddressComparator, true>;
      using RecorderType = Recorder<sizeof(__FILE__) + __LINE__>;
      template<class Derived> struct Leaf : BasicState<Derived, StatePolicy, true, true, true>, SingletonCreatorFake<Derived> {};
      template<class Derived, typename Statemachine> struct Composite : SubstatesHolderState<Derived, StatePolicy, Statemachine, true, true>, SingletonCreatorFake<Derived> {};

      struct InitialStateFake : StatePolicy {
        using Policy = StatePolicy;
        static const char* name;
      };
      const char* InitialStateFake::name = "Initial";

      template<class T>
      struct EmptyStateFake : T {
        static const char* name;

        using CreatorType = EmptyStateFake<T>;
        using ObjectType = EmptyStateFake<T>;

        static EmptyStateFake* create() { return nullptr; }
        static void destroy(EmptyStateFake*) { }

        template<class Event> void _entry(const Event&) { }
        template<class Event> bool _doit(const Event&) { return false; }
      };
      template<class T> const char* EmptyStateFake<T>::name = "Final";

      namespace Trigger
      {
        struct Next {};
        struct Count {};
        struct Check {};
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
        int counter = 0;

        template<class Event> void entry(const Event&) { RecorderType::add("C::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("C::Exit"); }
        template<class Event> void doit(const Event&) {
          RecorderType::add("C::Do");
          if (is_same<Event, Trigger::Count>().value) {
            counter++;
          }
        }
      };
      const char* C::name = "C";

      struct CheckTriggerGuardC {
        template<class StateType, class EventType>
        bool eval(const StateType& state, const EventType& ev) {
          return state.counter > 3;
        }
      };

      using ToBfromA = Transition<Trigger::Next, B, A, NoGuard, ActionSpy<B, A, RecorderType>>;
      using ToCfromB = Transition<Trigger::Next, C, B, NoGuard, ActionSpy<C, B, RecorderType>>;
      using ToCfromC = SelfTransition<Trigger::Count, C, NoGuard, ActionSpy<C, C, RecorderType>, false>;
      using ToFinalFromB = FinalTransition<B>;
      using ToFinalFromC = FinalTransitionExplicit<Trigger::Check, C, CheckTriggerGuardC, NoAction>;
      using TransitionList =
        Typelist<ToBfromA,
        Typelist<ToCfromB,
        Typelist<ToCfromC,
        Typelist<ToFinalFromB,
        Typelist<ToFinalFromC,
        NullType>>>>>;
      using InitTransition = InitialTransition<A, ActionSpy<A, InitialStateFake, RecorderType>>;
      using Sm = Statemachine<TransitionList, InitTransition>;
    }

    BEGIN(
      InitFinishTransitionTest,
      "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=InitFinishTransitionTest.drawio#R7ZpZc6M4EIB%2Fjat2HpziMD4ebWJnU5VMsnE210tKBhmYCMQIOT5%2B%2FbaMMKePccixVXGqYqmRGtT9qbsBN3TTX5wxFLqX1MakoSn2oqGfNjRNVRQFvoRkGUvavW4scJhny0GpYOytcDJTSmeejaPcQE4p4V6YF1o0CLDFczLEGJ3nh00pyZ81RA4uCcYWImXpvWdzN5Z2DSWV%2F409x03OrCYL9lEyWAoiF9l0nhHpw4ZuMkp53PIXJibCeIld4nmjLUc3F8ZwwA%2BZwOfXt5gs%2BbB%2FPl8Ew9X167DblFpeEZnJBUe%2BvFy%2BTGwAVx6K5swnI4Z8aA7mrsfxOESWkM%2FB9SBzuU%2Bgp0KT0VlgY1v2NitXoGNR37Nkm6AJJgNkvTjrCSYllMGhgAbiHBFn9GVjdaFoSgM%2BQr5HBEx3mNkoQFIsyVHBrANEPCeAjgWWwaBwUDZVsm7MOF5kRNJ0Z5j6mLMlDJFHdenFZb47T5lQlQQKNwOEkYxEEkRnozr1FTSku%2F7AdVrJdf2S5%2FJ%2B2OM0xCxpRa0lrOoRknikoelT%2BFjKxi25I4rY5kf7%2BUCvin7mrIoiz1rytnCqBzu4Lw9wGtbDgNrOQ6BqZQo0pQICXXsvCPQSBINvCN4Vglb3y0HQLbkc25DEZJcy7lKHBogMU2kmQgvzpWMuqDDUGoVfmPOlND6acZoHZaspIzpjFt5xuT2ZxhFzMN8f4MRadjqGYYK495pP2LUbuVfaaWWrEwJlCc7bKUmeEayXjzniuLytNhhv31a1ZNS6N5ntMai5PCqE4HaB%2F3vE2VZ5h%2BlVG6yG%2FaX%2FY0c%2FnfP%2BE79aXZw%2FT2%2BfHsyKImkirnbM8RQF0Dhjng9VEyhX%2FjJ%2FwH%2F40%2BM%2B8cBYEfhNU4T%2FmODexfDfFEcHj80xJG2ldaKIuZ4wayD2hBC6nIsaty%2BuXRtZDK8hB2f7NIhOKHNAKtVH0JwsmxGC77Wu0Y8SnGB7XiAzB1uCSIZMKUrcTvCUV0RW37Ntsq0mzIeZDGi9dynLjE4ZFaMqGNcRi5%2Bvns8jbeKuwpunc9dl%2FV9mu4KVW%2BY5DmaxK9tEmHACHLQd0fopvFKSmmA1%2FnEenFDOqX%2BQB%2BtIoIXNXbG71SqXqWod%2B7t3d3n3%2B9%2BH8FnpXF34K9yaLyt89mehHQf28YH9nWKo0TbyO0P5uCA6ftJUehle318Mb1oBv7tfdJzEyB9bpID92PJBzl93HkXnxEi6p4vswdNl0lt4%2FCEdCb3HRCO000mik8w5viDSywXRLk7rK4jk1GvqiXCzialqYYd2jLyKeEFyVvaJQ0FRq9vLKdKVAlzxikuK1nxt1nM8cuU75DjcfgKHkic1Q5Nyohk7gdoGr7aP3uNJ1A4szfUDSZQgNJUTpd3u5GBIIHsjrFqnAKtRYOxQWIuKjGK62QJrqigZSKfTCL8VaIy0Ph6MzroPK2vaHE2i67OgIlGZJZq%2F7%2FYbNd7td5XPvNuvhKAc1f7%2F1Uq3%2B3nVSqWRy8%2FVTBdbLyBqGMD5bM2dcHEb%2BQK1ddkOAr1hnH4XOnviVza97EJ8b3pJ3inVk0qMXk2ppK19%2FVTS%2Bq6N3l6l74L8C9RGeqGk0YocHlzI9%2FYo%2BligjYFq%2FWYLZnV71GyGN3yGUVVtVPkUBdJfX7wXTouRyveWuyA7OKjmiN4N9EFcVi68omaviUsVPjm%2FN1WjlJYrX3rWw2%2BvUHxpxwbkrqrtVrSFXwAFLTPDQjEg%2BizCDyj89pX7URj%2FRmLqLQTltZRuhYf1WqfMiFrByBFP86Cb%2Flwitmr6oxN9%2BB8%3D")

      INIT(
        Initialize,
        {
          using namespace InitFinishTransitionTestImpl;
          RecorderType::reset();

          SingletonCreatorFake<A>::reset();
          SingletonCreatorFake<B>::reset();
          SingletonCreatorFake<C>::reset();
        })

      TEST(
        Begin,
        InitStateA,
        ActiveStateA)
      {
        using namespace InitFinishTransitionTestImpl;
        Sm sm;
        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        // Active state is A
        NEQ(0, SingletonCreatorFake<A>::createCalls);
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
        EQ(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
      }

      TEST(
        End,
        ActiveStateWithoutEndTransition,
        ExitNotCalled)
      {
        using namespace InitFinishTransitionTestImpl;
        Sm sm;
        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "A::Do" });

        sm.end();
        RecorderType::checkUnchanged();

        // Active state is A
        NEQ(0, SingletonCreatorFake<A>::createCalls);
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
        EQ(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
      }

      TEST(
        End,
        ActiveStateWithEndTransition,
        ExitCalled)
      {
        using namespace InitFinishTransitionTestImpl;
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

        RecorderType::reset();

        sm.end();
        RecorderType::check({
          "B::Exit" });

        // States A, B and C are deactivated
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
        EQ(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
      }

      TEST(
        Final,
        ActiveStateFinalConditionMet,
        ExitCalled)
      {
        using namespace InitFinishTransitionTestImpl;
        Sm sm;
        sm.begin();

        // to B from A
        sm.dispatch<Trigger::Next>();

        // to C from B
        auto result = sm.dispatch<Trigger::Next>();

        result = sm.dispatch<Trigger::Count>();
        result = sm.dispatch<Trigger::Count>();
        result = sm.dispatch<Trigger::Count>();
        result = sm.dispatch<Trigger::Count>();

        // count is 4

        RecorderType::reset();

        result = sm.dispatch<Trigger::Check>();

        RecorderType::check({
          "C::Exit" });

        // States A, B and C are deactivated
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
        EQ(SingletonCreatorFake<C>::createCalls, SingletonCreatorFake<C>::deleteCalls);
      }

    END

  }
}
