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

    namespace InitFinishTransitionSubstateTestImpl {

      using StatePolicy = State<MemoryAddressComparator, true>;
      using RecorderType = Recorder<sizeof(__FILE__) + __LINE__>;
      template<class Derived> struct Leaf : BasicState<Derived, StatePolicy, true, true, true>, SingletonCreatorFake<Derived> {};
      template<class Derived, class Statemachine> struct Composite : SubstatesHolderState<Derived, StatePolicy, Statemachine, true, true>, SingletonCreatorFake<Derived> {};

      struct InitialStateFake : StatePolicy {
        using Policy = StatePolicy;
        static const char* name;
      };
      const char* InitialStateFake::name = "*";

      namespace Trigger
      {
        struct B_A {};
        struct A_B {};
        struct B_AA {};
        struct B_AAB {};
        struct AAB_AAA {};
      }

      struct B : Leaf<B> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("B::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("B::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("B::Do"); }
      };
      const char* B::name = "B";

      struct AAA : Leaf<AAA> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("AAA::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("AAA::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("AAA::Do"); }
      };
      const char* AAA::name = "AAA";

      struct AAB : Leaf<AAB> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("AAB::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("AAB::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("AAB::Do"); }
      };
      const char* AAB::name = "AAB";

      using ToAABfromAAA = Transition<Trigger::AAB_AAA, AAB, AAA, NoGuard, ActionSpy<AAB, AAA, RecorderType>>;
      using ToBfromAAB = ExitTransition<Trigger::B_AAB, B, AAB, NoGuard, ActionSpy<B, AAB, RecorderType>>;
      using ToFinalFromAAA = FinalTransition<AAA>;
      using ToFinalFromAAB = FinalTransition<AAB>;
      using TransitionsAA =
        Typelist<ToAABfromAAA,
        Typelist<ToBfromAAB,
        Typelist<ToFinalFromAAA,
        Typelist<ToFinalFromAAB,
        NullType>>>>;
      using InitAA = InitialTransition<AAA, ActionSpy<AAA, InitialStateFake, RecorderType>>;
      using SmAA = Statemachine<TransitionsAA, InitAA>;

      struct AA : Composite<AA, SmAA> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("AA::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("AA::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("AA::Do"); }
      };
      const char* AA::name = "AA";

      using ToBfromAA = ExitTransition<Trigger::B_AA, B, AA, NoGuard, ActionSpy<B, AA, RecorderType>>;
      using ToFinalFromAA = FinalTransition<AA>;
      using DeclToAABfromAAA1 = Declaration<Trigger::AAB_AAA, AA>;
      using DeclToBfromAAB1 = Declaration<Trigger::B_AAB, AA>;
      using TransitionsA =
        Typelist<ToBfromAA,
        Typelist<ToFinalFromAA,
        Typelist<DeclToAABfromAAA1,
        Typelist<DeclToBfromAAB1,
        NullType>>>>;
      using InitA = InitialTransition<AA, ActionSpy<AA, InitialStateFake, RecorderType>>;
      using SmA = Statemachine<TransitionsA, InitA>;

      struct A : Composite<A, SmA> {
        static const char* name;
        template<class Event> void entry(const Event&) { RecorderType::add("A::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("A::Exit"); }
        template<class Event> void doit(const Event&) { RecorderType::add("A::Do"); }
      };
      const char* A::name = "A";

      using ToBfromA = Transition<Trigger::B_A, B, A, NoGuard, ActionSpy<B, A, RecorderType>>;
      using ToAfromB = Transition<Trigger::A_B, A, B, NoGuard, ActionSpy<A, B, RecorderType>>;
      using ToFinalFromA = FinalTransition<A>;
      using ToFinalFromB = FinalTransition<B>;
      using DeclToAABfromAAA = Declaration<Trigger::AAB_AAA, A>;
      using DeclToBfromAA = ExitDeclaration<Trigger::B_AA, B, A>;
      using DeclToBfromAAB = ExitDeclaration<Trigger::B_AAB, B, A>;
      using Transitions =
        Typelist<ToBfromA,
        Typelist< ToAfromB,
        Typelist<ToFinalFromA,
        Typelist<ToFinalFromB,
        Typelist<DeclToAABfromAAA,
        Typelist<DeclToBfromAA,
        Typelist<DeclToBfromAAB,
        NullType>>>>>>>;
      using InitTransition = InitialTransition<A, ActionSpy<A, InitialStateFake, RecorderType>>;
      using Sm = Statemachine<Transitions, InitTransition>;
    }

    BEGIN(
      InitFinishTransitionSubstateTest,
      "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=InitFinishTransitionSubstateTest.drawio#R7Vttc5s4EP41nrl%2BSIdXY3%2B0HaeXGfea1r00yZcMNjLQAPIJpbbz60%2ByxYuQbBMCOO04mUmkRRKw%2B%2BzuowU6%2Bihcf0L20vsMHRB0NMVZd%2FTLjqapiqKQf1Sy2Um6%2Fd5O4CLfYYMywdR%2FAclMJn32HRBzAzGEAfaXvHAOowjMMSezEYIrftgCBvxZl7YLBMF0bgei9IfvYG8n7ZlKJv8b%2BK6XnFlNbji0k8FMEHu2A1c5kT7u6CMEId61wvUIBFR5iV528672HE0vDIEIl5mAVzffQbDB48H1ah2NX25%2BjXsXbJVfdvDMbjgO2eXiTaIDcuVL2nwOgytkh6Q5XHk%2BBtOlPafyFTE9kXk4DEhPJU0EnyMHOKyX3rlCOnMY%2BnPWDuwZCIb2%2FMndThjBACJyKIIRPUeMEXxKtU4XWsAIX9mhH1Aw3QLk2JHNxAw5KlHr0A58NyKdOdEMIAsORVUl9w0QBuuciKnuE4AhwGhDhrCjOrPihu%2BuMkxYXSbzcngwLSa0GQ7ddOXMVKTBrPUKy2mC5QaC4XgzHLGZjeZMiZpBleoHQWKQjqYvyM9cSa3CHVGol1c2c0mj0n7urIrCzioYm9rUJw48YAcwXNYDAbXLY0DVRBBopgQEhtEUCHQBBMMzCBoFgdErAQJFAgJdawoEPcHkwCE5jHUhwh50YWQH40yaC9BUfdmYCaSK2kLhJ8B4w5RvP2PIA2WvKmP4jObgwOX2WRa3kQvw8QBH7%2BWgYRAIbOz%2F4vN17UruC54maj0ICCsBvJ6S3BmT%2B8VTbGMgulUK4%2F1uVUtCrdvJHB8RyuVDKiRmp%2FBvIs4aoofpMgerwb%2F0r078j3s9eMBfXibXj4vvD3cjCUea0audYrCwI9L4hPyQkCayuPLX6AP5S371XT%2FwibJiYjdNofZDFPceIH9H9Ojw%2FmJKkrZifFToXJ%2BqNaI%2BQYUexpTiDui1a1dzBLYgJ8YOYRR%2FhMglUrZ8TJqzzUVsk%2F%2Fbta4%2BCOAkuscFZHJgSyCSQyYTJWYPwAJLImvoO06wjxLyYSYHtH4jrKyrilAxZcG4jlj8%2BOXxOtZm3svy28O156HBz1FXgpXvyHddgHam7AZUhTOCg65LW8NHuXCPVCIeDNiBtuw9gxjDsJS960i3hVDQEw2sygysqnVEg%2F7t59v%2F%2Fr1bPirWl0n4AozVRmLh1yUCEDnV00BDEdfsmpyaU59pIeROHzQVfl7e%2FJiMvxkRvv2xttxEye1SGqI%2FtLlj87ede9r5aCbdy3X%2B4OUm6a19fJeNJL37ZEXSzibRTjKnOn3SRfp0CKf10Sc29QaSPJWLwGrBQy2TX2J3Q2xWvjxRWMjo9bmFdKUArt0dCwtt8ZXeTynI3avh5Gm9%2BHqtT9B4NTD8p5Ep8WtJVD3vpTp1bqiLe6meKUZ3S5a%2Be%2BZ%2BnJYNPFIUSKoqZxg0DANN4bNPWkPJwaAvq63VkH2kIBCrKoRlnUHQKgh068QgMN7I884b%2FvJ5oF%2FIA5LqelP0U2p78yT0U0ojU%2BYopZEZZc1Y6n3%2B2B7KWop%2BHnKLPP08RKeapp8pZWDQ6RUgUZZ9atYeCLbEPrvnaHOqaKO1uNmV2t46R5sjbnE02tT%2BrODPjja9N0ab91jDKhYidEktuFW37p%2FSrdW8U6cufsytX1f3qu7WWkm37rXj1sXHCGRXUdGvC0%2Bu0n5bNSzlt8Oc2hbmJHXTU2JOLxY9iiWtspjT9cLGufjko2nMiRvlfc%2BkWqU1WvlifI7XaC0Rm7IRUC%2BJRmb8C42RyTei09AKgUyvWtUvku2S6MwWSgbCxSIGzSBYfLeD4les%2BJ0sllrVErjVajQ99LSqbfxq%2FcIDTbVqRtd1biGjWG98B%2FhNfPXPovPFl4Ak9eBW6bxEqTVmK6tStuKSlXrE2Y9Hr%2BrhIHkz%2F%2Bg%2BXfnd2VXxzaE9%2Fj9AyN7khi3pgPjQBfd4uLMLzhC7W7HewHHSOjef4rRqKU6rJcUdev%2B0thT3VkgXAqJuditC2uIhbTQEacMockjl4HUJ4xVufEMuIBbet%2B%2FkSdxiQovfPJSPPh5NX45EIPZf7Nl2PQpLpjyyuDnsmJdSoB70WSGTpp9AsbN08l8ZyTLsBckfBg%2BpWriXxa15YWj8Ag3yoJMW0t93xU2SoE8ZzIT9ZfELmab3l7VhTiyl717ffb8BxHrvAcQoRJBkf95CBHnrVx7vcSeVblmTvbApvmXX6k5KP02Vuuq2qcoerVSgLss6D0K16UhtFgKsUAApG6mLC5klI%2FWrd1IGv5NKvsCtSCNJN%2FtEezc8%2B9BdH%2F8P")

      INIT(
        Initialize,
        {
          using namespace InitFinishTransitionSubstateTestImpl;
          RecorderType::reset();

          SingletonCreatorFake<A>::reset();
          SingletonCreatorFake<AA>::reset();
          SingletonCreatorFake<AAA>::reset();
          SingletonCreatorFake<AAB>::reset();
          SingletonCreatorFake<B>::reset();
        })

      TEST(
        Begin,
        InitStateA,
        ActiveStateAAA)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        Sm sm;
        sm.begin();
        RecorderType::check({
          "A<-*",
          "A::Entry",
          "AA<-*",
          "AA::Entry",
          "AAA<-*",
          "AAA::Entry",
          "AAA::Do" });

        // Active state is A/AA/AAA
        NEQ(0, SingletonCreatorFake<A>::createCalls);
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
        EQ(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls + 1);
        EQ(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls + 1);
        EQ(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
      }

      TEST(
        End,
        ThirdLevel,
        ExitIsCalled)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        Sm sm;
        sm.begin();

        RecorderType::reset();

        sm.end();
        RecorderType::check({
          "AAA::Exit",
          "AA::Exit",
          "A::Exit" });

        // States A and B are deactivated
        NEQ(0, SingletonCreatorFake<A>::createCalls);
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        EQ(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        EQ(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        EQ(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
      }

      TEST(
        Dispatch,
        TransitionOnTopLevel,
        ThenExitIsCalled)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        Sm sm;
        sm.begin();

        RecorderType::reset();
        sm.dispatch<Trigger::B_A>();

        RecorderType::check({
          "B<-A",
          "AAA::Exit",
          "AA::Exit",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        EQ(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        EQ(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        EQ(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
      }

      TEST(
        Dispatch,
        TransitionSecondToTopLevel,
        ExitIsCalled)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        Sm sm;
        sm.begin();

        RecorderType::reset();
        sm.dispatch<Trigger::B_AA>();

        //vector<string>* localRecorder = &RecorderType::records_;

        RecorderType::check({
          "B<-AA",
          "AAA::Exit",
          "AA::Exit",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        EQ(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        EQ(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        EQ(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
      }

      TEST(
        Dispatch,
        TransitionThirdToTopLevel,
        ExitIsCalled)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        Sm sm;
        sm.begin();

        sm.dispatch<Trigger::AAB_AAA>();

        RecorderType::reset();
        sm.dispatch<Trigger::B_AAB>();

        RecorderType::check({
          "B<-AAB",
          "AAB::Exit",
          "AA::Exit",
          "A::Exit",
          "B::Entry",
          "B::Do" });

        // Active state is B
        EQ(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        EQ(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        EQ(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        EQ(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        EQ(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
      }

    END

  }
}
