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
#define IAMWORKSTATION 1

#include "CppUnitTest.h"

#include "..\..\src\state.h"
#include "..\..\src\lokilight.h"
#include "..\..\src\statemachine.h"
#include "..\..\src\transition.h"
#include "TestHelpers.h"

namespace UT {
  namespace Transitions {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace InitFinishTransitionSubstateTestImpl {

      typedef State<MemoryAddressComparator, true> StateType;
      typedef SingletonCreatorFake<StateType> StateTypeCreationPolicyType;
      typedef Recorder<sizeof(__FILE__) + __LINE__> RecorderType;
      template<class Derived> struct Leaf : BasicState<Derived, StateType>, SingletonCreatorFake<Derived> {};
      template<class Derived, class Statemachine> struct Composite : SubstatesHolderState<Derived, StateType, Statemachine>, SingletonCreatorFake<Derived> {};

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

        template<class Event>
        void _entry() { }
        template<class Event>
        bool _doit() { return false; }
      };
      template<class T> const char* EmptyStateFake<T>::name = "Final";

      namespace Trigger
      {
        struct B_A;
        struct B_AA;
        struct B_AAB;
        struct AAB_AAA;
      }

      struct A;
      struct B;
      struct AA;
      struct AAB;
      typedef ExitTransition<Trigger::B_AA, B, AA, StateTypeCreationPolicyType, NoGuard, ActionSpy<B, AA, RecorderType>> ToBfromAA;
      typedef FinalTransition<AA, StateTypeCreationPolicyType> ToFinalFromAA;
      typedef Declaration<Trigger::AAB_AAA, AA, StateTypeCreationPolicyType> DeclToAABfromAAA1;
      typedef Declaration<Trigger::B_AAB, AA, StateTypeCreationPolicyType> DeclToBfromAAB1;
      typedef
        Typelist<ToBfromAA,
        Typelist<ToFinalFromAA,
        Typelist<DeclToAABfromAAA1,
        Typelist<DeclToBfromAAB1,
        NullType>>>> TransitionsA;
      typedef InitialTransition<AA, StateTypeCreationPolicyType, ActionSpy<AA, InitialStateFake, RecorderType>> InitA;
      typedef Statemachine<
        TransitionsA,
        InitA> SmA;

      struct A : Composite<A, SmA> {
        static const char* name;
        void entry() { RecorderType::add("A::Entry"); }
        void exit() { RecorderType::add("A::Exit"); }
        template<class Event>
        void doit() { RecorderType::add("A::Do"); }
      };
      const char* A::name = "A";

      struct AAA;
      struct AAB;
      typedef Transition<Trigger::AAB_AAA, AAB, AAA, StateTypeCreationPolicyType, NoGuard, ActionSpy<AAB, AAA, RecorderType>> ToAABfromAAA;
      typedef ExitTransition<Trigger::B_AAB, B, AAB, StateTypeCreationPolicyType, NoGuard, ActionSpy<B, AAB, RecorderType>> ToBfromAAB;
      typedef FinalTransition<AAA, StateTypeCreationPolicyType> ToFinalFromAAA;
      typedef FinalTransition<AAB, StateTypeCreationPolicyType> ToFinalFromAAB;
      typedef
        Typelist<ToAABfromAAA,
        Typelist<ToBfromAAB,
        Typelist< ToFinalFromAAA,
        Typelist<ToFinalFromAAB,
        NullType>>>> TransitionsAA;
      typedef InitialTransition<AAA, StateTypeCreationPolicyType, ActionSpy<AAA, InitialStateFake, RecorderType>> InitAA;
      typedef Statemachine<
        TransitionsAA,
        InitAA> SmAA;

      struct AA : Composite<AA, SmAA> {
        static const char* name;
        void entry() { RecorderType::add("AA::Entry"); }
        void exit() { RecorderType::add("AA::Exit"); }
        template<class Event>
        void doit() { RecorderType::add("AA::Do"); }
      };
      const char* AA::name = "AA";

      struct AAA : Leaf<AAA> {
        static const char* name;
        void entry() { RecorderType::add("AAA::Entry"); }
        void exit() { RecorderType::add("AAA::Exit"); }
        template<class Event>
        void doit() { RecorderType::add("AAA::Do"); }
      };
      const char* AAA::name = "AAA";

      struct AAB : Leaf<AAB> {
        static const char* name;
        void entry() { RecorderType::add("AAB::Entry"); }
        void exit() { RecorderType::add("AAB::Exit"); }
        template<class Event>
        void doit() { RecorderType::add("AAB::Do"); }
      };
      const char* AAB::name = "AAB";

      struct B : Leaf<B> {
        static const char* name;
        void entry() { RecorderType::add("B::Entry"); }
        void exit() { RecorderType::add("B::Exit"); }
        template<class Event>
        void doit() { RecorderType::add("B::Do"); }
      };
      const char* B::name = "B";

      typedef Transition<Trigger::B_A, B, A, StateTypeCreationPolicyType, NoGuard, ActionSpy<B, A, RecorderType>> ToBfromA;
      typedef FinalTransition<A, StateTypeCreationPolicyType> ToFinalFromA;
      typedef FinalTransition<B, StateTypeCreationPolicyType> ToFinalFromB;
      typedef Declaration<Trigger::AAB_AAA, A, StateTypeCreationPolicyType> DeclToAABfromAAA;
      typedef ExitDeclaration<Trigger::B_AA, B, A, StateTypeCreationPolicyType> DeclToBfromAA;
      typedef ExitDeclaration<Trigger::B_AAB, B, A, StateTypeCreationPolicyType> DeclToBfromAAB;
      typedef
        Typelist<ToBfromA,
        Typelist< ToFinalFromA,
        Typelist<ToFinalFromB,
        Typelist<DeclToAABfromAAA,
        Typelist<DeclToBfromAA,
        Typelist<DeclToBfromAAB,
        NullType>>>>>> Transitions;

      typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateFake, RecorderType>> InitTransition;
      typedef Statemachine<
        Transitions,
        InitTransition> Sm;
    }

    // https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=InitFinishTransitionSubstateTest.drawio#R7Vttc5s4EP41nrl%2BSIdXY3%2B0HaeXGfea1r00yZcMNjLQAPIJpbbz60%2ByxYuQbBMCOO04mUmkRRKw%2B%2BzuowU6%2Bihcf0L20vsMHRB0NMVZd%2FTLjqapiqKQf1Sy2Um6%2Fd5O4CLfYYMywdR%2FAclMJn32HRBzAzGEAfaXvHAOowjMMSezEYIrftgCBvxZl7YLBMF0bgei9IfvYG8n7ZlKJv8b%2BK6XnFlNbji0k8FMEHu2A1c5kT7u6CMEId61wvUIBFR5iV528672HE0vDIEIl5mAVzffQbDB48H1ah2NX25%2BjXsXbJVfdvDMbjgO2eXiTaIDcuVL2nwOgytkh6Q5XHk%2BBtOlPafyFTE9kXk4DEhPJU0EnyMHOKyX3rlCOnMY%2BnPWDuwZCIb2%2FMndThjBACJyKIIRPUeMEXxKtU4XWsAIX9mhH1Aw3QLk2JHNxAw5KlHr0A58NyKdOdEMIAsORVUl9w0QBuuciKnuE4AhwGhDhrCjOrPihu%2BuMkxYXSbzcngwLSa0GQ7ddOXMVKTBrPUKy2mC5QaC4XgzHLGZjeZMiZpBleoHQWKQjqYvyM9cSa3CHVGol1c2c0mj0n7urIrCzioYm9rUJw48YAcwXNYDAbXLY0DVRBBopgQEhtEUCHQBBMMzCBoFgdErAQJFAgJdawoEPcHkwCE5jHUhwh50YWQH40yaC9BUfdmYCaSK2kLhJ8B4w5RvP2PIA2WvKmP4jObgwOX2WRa3kQvw8QBH7%2BWgYRAIbOz%2F4vN17UruC54maj0ICCsBvJ6S3BmT%2B8VTbGMgulUK4%2F1uVUtCrdvJHB8RyuVDKiRmp%2FBvIs4aoofpMgerwb%2F0r078j3s9eMBfXibXj4vvD3cjCUea0audYrCwI9L4hPyQkCayuPLX6AP5S371XT%2FwibJiYjdNofZDFPceIH9H9Ojw%2FmJKkrZifFToXJ%2BqNaI%2BQYUexpTiDui1a1dzBLYgJ8YOYRR%2FhMglUrZ8TJqzzUVsk%2F%2Fbta4%2BCOAkuscFZHJgSyCSQyYTJWYPwAJLImvoO06wjxLyYSYHtH4jrKyrilAxZcG4jlj8%2BOXxOtZm3svy28O156HBz1FXgpXvyHddgHam7AZUhTOCg65LW8NHuXCPVCIeDNiBtuw9gxjDsJS960i3hVDQEw2sygysqnVEg%2F7t59v%2F%2Fr1bPirWl0n4AozVRmLh1yUCEDnV00BDEdfsmpyaU59pIeROHzQVfl7e%2FJiMvxkRvv2xttxEye1SGqI%2FtLlj87ede9r5aCbdy3X%2B4OUm6a19fJeNJL37ZEXSzibRTjKnOn3SRfp0CKf10Sc29QaSPJWLwGrBQy2TX2J3Q2xWvjxRWMjo9bmFdKUArt0dCwtt8ZXeTynI3avh5Gm9%2BHqtT9B4NTD8p5Ep8WtJVD3vpTp1bqiLe6meKUZ3S5a%2Be%2BZ%2BnJYNPFIUSKoqZxg0DANN4bNPWkPJwaAvq63VkH2kIBCrKoRlnUHQKgh068QgMN7I884b%2FvJ5oF%2FIA5LqelP0U2p78yT0U0ojU%2BYopZEZZc1Y6n3%2B2B7KWop%2BHnKLPP08RKeapp8pZWDQ6RUgUZZ9atYeCLbEPrvnaHOqaKO1uNmV2t46R5sjbnE02tT%2BrODPjja9N0ab91jDKhYidEktuFW37p%2FSrdW8U6cufsytX1f3qu7WWkm37rXj1sXHCGRXUdGvC0%2Bu0n5bNSzlt8Oc2hbmJHXTU2JOLxY9iiWtspjT9cLGufjko2nMiRvlfc%2BkWqU1WvlifI7XaC0Rm7IRUC%2BJRmb8C42RyTei09AKgUyvWtUvku2S6MwWSgbCxSIGzSBYfLeD4les%2BJ0sllrVErjVajQ99LSqbfxq%2FcIDTbVqRtd1biGjWG98B%2FhNfPXPovPFl4Ak9eBW6bxEqTVmK6tStuKSlXrE2Y9Hr%2BrhIHkz%2F%2Bg%2BXfnd2VXxzaE9%2Fj9AyN7khi3pgPjQBfd4uLMLzhC7W7HewHHSOjef4rRqKU6rJcUdev%2B0thT3VkgXAqJuditC2uIhbTQEacMockjl4HUJ4xVufEMuIBbet%2B%2FkSdxiQovfPJSPPh5NX45EIPZf7Nl2PQpLpjyyuDnsmJdSoB70WSGTpp9AsbN08l8ZyTLsBckfBg%2BpWriXxa15YWj8Ag3yoJMW0t93xU2SoE8ZzIT9ZfELmab3l7VhTiyl717ffb8BxHrvAcQoRJBkf95CBHnrVx7vcSeVblmTvbApvmXX6k5KP02Vuuq2qcoerVSgLss6D0K16UhtFgKsUAApG6mLC5klI%2FWrd1IGv5NKvsCtSCNJN%2FtEezc8%2B9BdH%2F8P
    TEST_CLASS(InitFinishTransitionSubstateTest)
    {
    public:
      TEST_METHOD_INITIALIZE(Initialize)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        RecorderType::reset();

        SingletonCreatorFake<A>::reset();
        SingletonCreatorFake<AA>::reset();
        SingletonCreatorFake<AAA>::reset();
        SingletonCreatorFake<AAB>::reset();
        SingletonCreatorFake<B>::reset();
      }

      TEST_METHOD(Begin_WhenInitStateA_ThenActiveStateAAA)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        Sm sm;
        sm.begin();
        RecorderType::check({
          "A<-Initial",
          "A::Entry",
          "AA<-Initial",
          "AA::Entry",
          "AAA<-Initial",
          "AAA::Entry",
          "AAA::Do" });

        // Active state is A/AA/AAA
        Assert::AreNotEqual<int>(0, SingletonCreatorFake<A>::createCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls + 1);
        Assert::AreEqual<int>(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
      }

      TEST_METHOD(End_WhenThirdLevel_ThenExitIsCalled)
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
        Assert::AreNotEqual<int>(0, SingletonCreatorFake<A>::createCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls);
      }

      TEST_METHOD(Dispatch_WhenTransitionOnTopLevel_ThenExitIsCalled)
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
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
      }

      TEST_METHOD(Dispatch_WhenTransitionSecondToTopLevel_ThenExitIsCalled)
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
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
      }

      TEST_METHOD(Dispatch_WhenTransitionThirdToTopLevel_ThenExitIsCalled)
      {
        using namespace InitFinishTransitionSubstateTestImpl;
        Sm sm;
        sm.begin();

        auto result = sm.dispatch<Trigger::AAB_AAA>();

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
        Assert::AreEqual<int>(SingletonCreatorFake<A>::createCalls, SingletonCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AA>::createCalls, SingletonCreatorFake<AA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAA>::createCalls, SingletonCreatorFake<AAA>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<AAB>::createCalls, SingletonCreatorFake<AAB>::deleteCalls);
        Assert::AreEqual<int>(SingletonCreatorFake<B>::createCalls, SingletonCreatorFake<B>::deleteCalls + 1);
      }
    };
  }
}
