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
  namespace Classes {

    using namespace tsmlib;
    using namespace LokiLight;
    using namespace UnitTests::Helpers;

    namespace SubstatemachineEventTestImpl {

      using StatePolicy = State<VirtualTypeIdComparator, false>;

      using ToOffFromInitialActionSpy = ActionStub<struct OffState, struct EmptyState<StatePolicy>>;
      using ToFinalFromOffActionSpy = ActionStub<EmptyState<StatePolicy>, struct OffState>;
      using ToFinalFromOnActionSpy = ActionStub<EmptyState<StatePolicy>, struct OnState>;

      namespace Event {
        struct On { int id = 100; };
        struct Off { int id = 200; };
        struct Stop { int id = 300; };
        struct Start { int id = 400; };
      }

      struct Idle : BasicState<Idle, StatePolicy, true, true, true>, FactoryCreator<Idle> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<Idle, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int Idle::entryCalls = 0;
      int Idle::exitCalls = 0;
      int Idle::doitCalls = 0;

      struct OnState : BasicState<OnState, StatePolicy, true, true, true>, FactoryCreator<OnState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;
        static int lastEntryEvent;
        static int lastExitEvent;
        static int lastDoitEvent;

        uint8_t getTypeId() const override { return 1; }

        static void reset() {
          entryCalls = 0;
          exitCalls = 0;
          doitCalls = 0;
          lastEntryEvent = -1;
          lastExitEvent = -1;
          lastDoitEvent = -1;
        }

      private:
        friend class BasicState<OnState, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event& ev) { entryCalls++; lastEntryEvent = ev.id; }
        template<> void entry(const NullType& ev) { entryCalls++; lastEntryEvent = 0; }
        template<class Event> void exit(const Event& ev) { exitCalls++; lastExitEvent = ev.id; }
        template<> void exit(const NullType&) { exitCalls++; lastEntryEvent = 0; }
        template<class Event> void doit(const Event& ev) { doitCalls++; lastDoitEvent = ev.id; }
        template<> void doit(const NullType&) { doitCalls++; lastEntryEvent = 0; }
      };
      int OnState::entryCalls = 0;
      int OnState::exitCalls = 0;
      int OnState::doitCalls = 0;
      int OnState::lastEntryEvent = -1;
      int OnState::lastExitEvent = -1;
      int OnState::lastDoitEvent = -1;

      struct OffState : BasicState<OffState, StatePolicy, true, true, true>, FactoryCreator<OffState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;
        static int lastEntryEvent;
        static int lastExitEvent;
        static int lastDoitEvent;

        uint8_t getTypeId() const override { return 2; }

        static void reset() {
          entryCalls = 0;
          exitCalls = 0;
          doitCalls = 0;
          lastEntryEvent = -1;
          lastExitEvent = -1;
          lastDoitEvent = -1;
        }

        template<class Event> void entry(const Event& ev) { entryCalls++; lastEntryEvent = ev.id; }
        template<> void entry(const NullType& ev) { entryCalls++; lastEntryEvent = 0; }
        template<class Event> void exit(const Event& ev) { exitCalls++; lastExitEvent = ev.id; }
        template<> void exit(const NullType&) { exitCalls++; lastExitEvent = 0; }
        template<class Event> void doit(const Event& ev) { doitCalls++; lastDoitEvent = ev.id; }
        template<> void doit(const NullType&) { doitCalls++; lastDoitEvent = 0; }
      };
      int OffState::entryCalls = 0;
      int OffState::exitCalls = 0;
      int OffState::doitCalls = 0;
      int OffState::lastEntryEvent;
      int OffState::lastExitEvent = -1;
      int OffState::lastDoitEvent = -1;

      using ToOnFromOffTransition = Transition<Event::On, OnState, OffState, NoGuard, NoAction>;
      using ToOffFromOnTransition = Transition<Event::Off, OffState, OnState, NoGuard, NoAction>;
      using ToIdleFromOnTransition = ExitTransition<Event::Stop, Idle, OnState, NoGuard, NoAction>;
      using ToFinalFromOff = FinalTransition<OffState>;
      using ToFinalFromOn = FinalTransition<OnState>;

      using ActivestateTransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToIdleFromOnTransition,
        Typelist<ToFinalFromOff,
        Typelist<ToFinalFromOn,
        NullType>>>>>;

      using ActivestateInitTransition = InitialTransition<OffState, NoAction>;
      using ActivestateStatemachine = Statemachine<ActivestateTransitionList, ActivestateInitTransition>;

      struct Active : SubstatesHolderState<Active, StatePolicy, ActivestateStatemachine, true, true>, FactoryCreator<Active> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 3; }

      private:
        friend class SubstatesHolderState<Active, StatePolicy, ActivestateStatemachine, true, true>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int Active::entryCalls = 0;
      int Active::exitCalls = 0;
      int Active::doitCalls = 0;

      using ToFinalSubstatesGuardDummy = GuardDummy<StatePolicy, AnyState<StatePolicy>, AnyState<StatePolicy>>;
      using ToFinalSubstatesActionSpy = ActionStub<AnyState<StatePolicy>, AnyState<StatePolicy>>;

      using ToOnFromOffSubDeclaration = Declaration<Event::On, Active>;
      using ToOffFromOnSubDeclaration = Declaration<Event::Off, Active>;
      using ToIdleFromOnDeclaration = ExitDeclaration<Event::Stop, Idle, Active>;

      using ToActiveFromIdleTransition = Transition<Event::Start, Active, Idle, NoGuard, NoAction>;
      using ToIdleFromActiveTransition = Transition<Event::Stop, Idle, Active, NoGuard, NoAction>;
      using ToFinalFromActive = FinalTransition<Active>;
      using ToFinalFromIdle = FinalTransition<Idle>;

      using TransitionList =
        Typelist<ToOnFromOffSubDeclaration,
        Typelist<ToOffFromOnSubDeclaration,
        Typelist<ToIdleFromOnDeclaration,
        Typelist<ToActiveFromIdleTransition,
        Typelist<ToIdleFromActiveTransition,
        Typelist<ToFinalFromActive,
        Typelist<ToFinalFromIdle, NullType>>>>>>>;

      using InitTransitionToIdle = InitialTransition<Idle, NoAction>;
      using Sm = Statemachine<TransitionList, InitTransitionToIdle>;

      using InitTransitionToActive = InitialTransition<Active, NoAction>;
      using SmBeginEnd = Statemachine<TransitionList, InitTransitionToActive>;
    }

    BEGIN(SubstatemachineEventTest)

      INIT(
        Initialize,
        {
          reset();
        })

      TEST(
        StateWithSubstates,
        Begin,
        CallsEntryAndEntryAndDoitOnStateAndSubstate,
        "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&page-id=vcinDnvGSfqwhQla_7vM&title=SubstatemachineTest.drawio#R%3Cmxfile%20pages%3D%222%22%3E%3Cdiagram%20id%3D%22YKBjTKGMOBF2p5uhFZQw%22%20name%3D%22Statemachine%22%3E7Vtdc9o4FP01mdk%2BJONv4BFokrLTbrIls%2F14yQhb2G5li5FFgPz6lbAEtmTAITahM0lngnUtX9v3nHt1dGku7GGyvCVgFn3BAUQXlhEsL%2ByPF5ZlGobBPrhllVu8Xjc3hCQOxKStYRw%2FQ3mlsM7jAGaliRRjRONZ2ejjNIU%2BLdkAIXhRnjbFqHzXGQihZhj7AOnWb3FAo9zadY2t%2FROMw0je2ZQvnAA5WRiyCAR4UTDZ1xf2kGBM86NkOYSIB0%2FGJb%2FuZsfZzYMRmNI6F9DF%2FQNEK3rdHy2W6fXz%2FdN191J4eQJoLl44S8Tj0pWMAXvyGT%2BcJ%2BiGgIQdDhZRTOF4BnxuXzDomS2iCWIjkx0SPE8DGIjR5s0NNvBxEvviGIEJRAPg%2Fw7XFwwxwoSdSnHK75FRgn9vos4dTXFKb0ASI06m%2FyAJQAqEWTDHZGEdABSHKRv4LDKQORzooZLvDQmFy4JJhO4W4gRSsmJTxFlboLgqDxcFTjjCFhX44MqJQPAw3HjeQsUOBFovQM7SkOv7NH6CGnplLA4AB4gvImk5PLIxQhKVC8uesh%2Ff2EBTOmPwVD8a65rI8nHhroYh7qohzoGNWRb3xQmKZ83wwHHKRDANV2OCY1Ywwe42wIT5l7v%2B6FMn8%2F52LwfeOBxPw1%2BX9nsO18Suo2DnOBp2tluFndFWFuvYjQL0nsPt5rCpFHPT0qu5VZnDXls8cDQe3E2n7zRolQauQgNm0WlgVNCg0xYLXJ0F6TsJTkoC27XfmARdDXEYsC2JGGJCIxziFKDrrbWwVvPobed8xjxOayb8gpSuROzBnOIyT1gAyeo7v%2F7KlcMfwt168HFZGq3EaCcCGZ4TH%2B55zV4%2BjwISQnp4heQx2IsngQis9W9pB9g0OD0tQ3W0EGKbU1iOr5RfGXtfOqaAQj0bN%2BzfnY2NaLKmczOICdt5x5gbGew8a9pYpR19kbYrtVpL0EtBcNrEPD7BTNkuOZRhTtMZtr60TwhYFSbMcJzSrOD5nhu2cHuqKOsYCmS5xy2Am0d7BaZ6%2F%2BM9n1vKZ9dVAHZ1udVWQtv%2FBtk%2F4aj%2Fk949fx49Th9%2Bfh9W9L4m%2FHHHFE4Bk13GLYkTtpFmzo2%2Fhh%2FYb%2FbPzscoZtHKGHCWwQEkPM0itm0zhvzs4MfluM8%2BnSuDXxvzuKY8BbkxopS3Lvv82a0bn8B1TjG0E5xmV5iEzCrcs2y5mawuM8A%2B175uPmjsZMGnCjVLbJMcKVBTmCTuCE5phcJK4iBAu9oE5bJWYFqvlW6b29Frv1slyqwGuPJ49zjKrEn0PPv6cxRFpP9r6FVw5YHEYQhJDqWHeAgnjAdeyI%2FGvCxUmVnV16xc42u2it1fa1BPMKU4qQV1A9gqDTSnYl03q7A1zQbArWyg6eBWbLtOIcKXMc01uOWK4Y%2FCqa0E5wOpwAvK3TqRdHdqCgu3prAQTNgorJcJDU1JdGVLdsMwp%2Bwif0FxVfEbE82RVy5DskEgHeUR0Bztlj76reQcPJ1msOTlCI1TyW79i4Kq4vKGe0yzNaK6DSvghonqdpUGVEfhV12iqo5s%2B08kqt4DzdfLnf2v1%2FLQLJXLQvncQURZny%2BNK8OwChWaXds5UKXZ4B4yQQnXGju3HUy545lv1WR%2B3e5Kw8y3PIWwajetLvM1R6pI2MH8lnh9P8bRIJv0%2BwB1fy8i%2Bu1xFVfIiye9%2FO7bZMI0OH6L2YBi62hBNjXJ1tbOrTKi%2BpJ2TuvZIeEly4hZqiAdd28BaVSt7aPpydWaQi5LzeC6paCnrqam8izntghWoqB%2FIXxO1D4k1aqpbbVF7Qp9d1bUVv6%2FgdaeqE3tnuroT6T2W%2Bu7Tl19Z5TYu5%2B8bYq4utuXuiJOsMm4Mk2nvLOVGfJKwjtaCT5yQ%2BMZqiPv7GSd%2FmX9HybrHKWquKbeiTuprPMOr32tFYfateHqBa25027w9rH01EufrW4Zut3jKoGjNPNs9%2FwqQefMNFv7zD6et%2FaZ81aRbHbvWN6aCm%2B9erw9L8kmo1rSbOuvtlqry0eyt7zjeDvNVsHvffW7vmbjXd0SoyTBXqvZnJ4itY5kvO2qmk1x1GqlZsPtn9Lk07d%2FkGRf%2Fw8%3D%3C%2Fdiagram%3E%3Cdiagram%20id%3D%22vcinDnvGSfqwhQla_7vM%22%20name%3D%22Begin%20End%22%3E7Vpbb%2BI4FP41SDsPrZw4AfoITOlIO6vdFaOd6dPKTUzw1ImRY1rYX782cUhsh5JhEjqV2krFPr7EOd93bqYDOEu3dxytV3%2BwGNOBD%2BLtAH4c%2BL4HAJAfSrIrJMObcSFIOIn1pEqwIP%2FhcqWWbkiMc2OiYIwKsjaFEcsyHAlDhjhnz%2Ba0JaPmU9cowY5gESHqSr%2BSWKwK6TgElfwTJsmqfLJXvnCKyslakK9QzJ5rIng7gDPOmCha6XaGqVJeqZdi3fzI6OFgHGeizYLPBMHF3de%2Fp0%2FfPj2uJ7%2BPs3%2B%2FXOldnhDd6BfOU31csSt1IE%2B%2BVs1NSuccpbI5fV4RgRdrFCn5s4ReylYipbLnySZnmyzGse4d3hzITsRSEuk2RQ%2BYTlH0mOwXzBhlXA5lLFPPyAVnjwetq42WLBNzlBKqyPQP5jHKkBZr5nhSrVNESZLJTiQ1g%2BWGU1dV5XtjLvC2JtKqu8MsxYLv5BQ9CjWKO7P7XONEoGWrGh%2FCciLSPEwOO1dQyYZG6weQ8x3kJpEgT9hBz8TiBHCIR1qTfqA0SygtURn4cCl%2FInCAxhgBytTPxrolsqpfeyoA%2BqkO4gpYIq14ogcEW3fDA883ieCB0GFC4DUwAY77YgJ8t%2BGW2IUWdkHgYAfDJuxAX9iFDnZ%2FLpfvJtyrCR9yC00D6aUdGviggQajvlgwdFmQvZPgoiSAIXxlEviuL3A4gGOZnOou42LFEpYheltJa15b6bOa85kpze258R0LsdNooI1gJnPwlohvavl1qHv3ejPV%2Fritd3a6cxSPnG14hF94aV0MCMQTLE5nO%2Br9X0SXY4r2WZBRBzRApZf%2BxYg8csWKwGRFGJo7FOfUiyy8D6c4nwLjFgygVJY%2B2MSsDO65PJ9YCCSwa%2FMHGztu851E%2FK49QEy4rOsIU0JJJ2WbneQCVkIfuAk9bMwEerL%2Bm9ew9fPttvSeJw037MVwJ5yjXW3CWllkftyuoeXtvRGwECt27NSay2e%2Bm3P%2F5mzX517o5nQXtWfPvVl5UOddCLxEMrkDd5ykskyTu4PfZh%2FkX%2FkLiz4lUl25RM4HCkGu7GwlC3swU6PT%2B6vFRH4G10CtJUqxmbJBJVwJoS7GJurw%2FjzieG9UEu6UZfk144mU6u2lucwfdlc5kp%2F7veYfHHpK7QuLmwbdSpLUuKlFJfAUL0VDHpeSOKbHilDTr9WodtPLXU44cn1%2F2JT6%2Bb2Rxb3M%2BcJJkmBeYDmkSocPkgjDRLVUdeDIGurG3uB7YEKwtBV8HeBlJWVBQ6z2mvDyvN4Ac%2B9cGiq2i2brfj1dL4ea03WJBN%2FVV6n%2BfX2wWrfv%2FXyeH7ZMF4Yt0wVNhUOM%2Fcm8PxiHFsUCc4viBZ3Ev2Gjoelb%2FHYVxPGExn1UOYctlznupQ7xApfeDe7lEvyuqGow1euNqcOOE9uOmeoPrcurkUWwtky1N4LwTTLVvTp7cnn6UpKNs%2Fj8FLuL4GZFN1hmjq%2BWuY4clf5Kln8qRpUR0auFQ3A9Cl%2BMiP0HNu%2BVIlto0cu3c6K2%2FmJoOx7POsvb8Bdtrth%2B3bDWTG6%2FL3K3jYWvRm7rCz2nQmtN7pG90Vskt%2B9eOL21YHhjleaeW%2BpdNBj67jWOo9EOvYER6CrXcMIbmF%2FVnKj9Tjq3892F39JdlDy9tLuAdso7Hp%2FnLgJgloswHLZyF904A9mt%2FmusmF797x28%2FR8%3D%3C%2Fdiagram%3E%3C%2Fmxfile%3E")
      {
        using namespace SubstatemachineEventTestImpl;
        SmBeginEnd sm;
        sm.begin();

        EQ(1, Active::entryCalls);
        EQ(0, Active::exitCalls);
        EQ(0, Active::doitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::doitCalls);
        EQ(1, OffState::entryCalls);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::doitCalls);

        EQ(-1, OnState::lastEntryEvent);
        EQ(-1, OnState::lastExitEvent);
        EQ(-1, OnState::lastDoitEvent);
        EQ(000, OffState::lastEntryEvent);
        EQ(-1, OffState::lastExitEvent);
        EQ(000, OffState::lastDoitEvent);
      }

      TEST(
        SubstateHasFinalStateDefined,
        End,
        CallsExitOnSubstateAndState,
        "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&page-id=vcinDnvGSfqwhQla_7vM&title=SubstatemachineTest.drawio#R%3Cmxfile%20pages%3D%222%22%3E%3Cdiagram%20id%3D%22YKBjTKGMOBF2p5uhFZQw%22%20name%3D%22Statemachine%22%3E7Vtdc9o4FP01mdk%2BJONv4BFokrLTbrIls%2F14yQhb2G5li5FFgPz6lbAEtmTAITahM0lngnUtX9v3nHt1dGku7GGyvCVgFn3BAUQXlhEsL%2ByPF5ZlGobBPrhllVu8Xjc3hCQOxKStYRw%2FQ3mlsM7jAGaliRRjRONZ2ejjNIU%2BLdkAIXhRnjbFqHzXGQihZhj7AOnWb3FAo9zadY2t%2FROMw0je2ZQvnAA5WRiyCAR4UTDZ1xf2kGBM86NkOYSIB0%2FGJb%2FuZsfZzYMRmNI6F9DF%2FQNEK3rdHy2W6fXz%2FdN191J4eQJoLl44S8Tj0pWMAXvyGT%2BcJ%2BiGgIQdDhZRTOF4BnxuXzDomS2iCWIjkx0SPE8DGIjR5s0NNvBxEvviGIEJRAPg%2Fw7XFwwxwoSdSnHK75FRgn9vos4dTXFKb0ASI06m%2FyAJQAqEWTDHZGEdABSHKRv4LDKQORzooZLvDQmFy4JJhO4W4gRSsmJTxFlboLgqDxcFTjjCFhX44MqJQPAw3HjeQsUOBFovQM7SkOv7NH6CGnplLA4AB4gvImk5PLIxQhKVC8uesh%2Ff2EBTOmPwVD8a65rI8nHhroYh7qohzoGNWRb3xQmKZ83wwHHKRDANV2OCY1Ywwe42wIT5l7v%2B6FMn8%2F52LwfeOBxPw1%2BX9nsO18Suo2DnOBp2tluFndFWFuvYjQL0nsPt5rCpFHPT0qu5VZnDXls8cDQe3E2n7zRolQauQgNm0WlgVNCg0xYLXJ0F6TsJTkoC27XfmARdDXEYsC2JGGJCIxziFKDrrbWwVvPobed8xjxOayb8gpSuROzBnOIyT1gAyeo7v%2F7KlcMfwt168HFZGq3EaCcCGZ4TH%2B55zV4%2BjwISQnp4heQx2IsngQis9W9pB9g0OD0tQ3W0EGKbU1iOr5RfGXtfOqaAQj0bN%2BzfnY2NaLKmczOICdt5x5gbGew8a9pYpR19kbYrtVpL0EtBcNrEPD7BTNkuOZRhTtMZtr60TwhYFSbMcJzSrOD5nhu2cHuqKOsYCmS5xy2Am0d7BaZ6%2F%2BM9n1vKZ9dVAHZ1udVWQtv%2FBtk%2F4aj%2Fk949fx49Th9%2Bfh9W9L4m%2FHHHFE4Bk13GLYkTtpFmzo2%2Fhh%2FYb%2FbPzscoZtHKGHCWwQEkPM0itm0zhvzs4MfluM8%2BnSuDXxvzuKY8BbkxopS3Lvv82a0bn8B1TjG0E5xmV5iEzCrcs2y5mawuM8A%2B175uPmjsZMGnCjVLbJMcKVBTmCTuCE5phcJK4iBAu9oE5bJWYFqvlW6b29Frv1slyqwGuPJ49zjKrEn0PPv6cxRFpP9r6FVw5YHEYQhJDqWHeAgnjAdeyI%2FGvCxUmVnV16xc42u2it1fa1BPMKU4qQV1A9gqDTSnYl03q7A1zQbArWyg6eBWbLtOIcKXMc01uOWK4Y%2FCqa0E5wOpwAvK3TqRdHdqCgu3prAQTNgorJcJDU1JdGVLdsMwp%2Bwif0FxVfEbE82RVy5DskEgHeUR0Bztlj76reQcPJ1msOTlCI1TyW79i4Kq4vKGe0yzNaK6DSvghonqdpUGVEfhV12iqo5s%2B08kqt4DzdfLnf2v1%2FLQLJXLQvncQURZny%2BNK8OwChWaXds5UKXZ4B4yQQnXGju3HUy545lv1WR%2B3e5Kw8y3PIWwajetLvM1R6pI2MH8lnh9P8bRIJv0%2BwB1fy8i%2Bu1xFVfIiye9%2FO7bZMI0OH6L2YBi62hBNjXJ1tbOrTKi%2BpJ2TuvZIeEly4hZqiAdd28BaVSt7aPpydWaQi5LzeC6paCnrqam8izntghWoqB%2FIXxO1D4k1aqpbbVF7Qp9d1bUVv6%2FgdaeqE3tnuroT6T2W%2Bu7Tl19Z5TYu5%2B8bYq4utuXuiJOsMm4Mk2nvLOVGfJKwjtaCT5yQ%2BMZqiPv7GSd%2FmX9HybrHKWquKbeiTuprPMOr32tFYfateHqBa25027w9rH01EufrW4Zut3jKoGjNPNs9%2FwqQefMNFv7zD6et%2FaZ81aRbHbvWN6aCm%2B9erw9L8kmo1rSbOuvtlqry0eyt7zjeDvNVsHvffW7vmbjXd0SoyTBXqvZnJ4itY5kvO2qmk1x1GqlZsPtn9Lk07d%2FkGRf%2Fw8%3D%3C%2Fdiagram%3E%3Cdiagram%20id%3D%22vcinDnvGSfqwhQla_7vM%22%20name%3D%22Begin%20End%22%3E7Vpbb%2BI4FP41SDsPrZw4AfoITOlIO6vdFaOd6dPKTUzw1ImRY1rYX782cUhsh5JhEjqV2krFPr7EOd93bqYDOEu3dxytV3%2BwGNOBD%2BLtAH4c%2BL4HAJAfSrIrJMObcSFIOIn1pEqwIP%2FhcqWWbkiMc2OiYIwKsjaFEcsyHAlDhjhnz%2Ba0JaPmU9cowY5gESHqSr%2BSWKwK6TgElfwTJsmqfLJXvnCKyslakK9QzJ5rIng7gDPOmCha6XaGqVJeqZdi3fzI6OFgHGeizYLPBMHF3de%2Fp0%2FfPj2uJ7%2BPs3%2B%2FXOldnhDd6BfOU31csSt1IE%2B%2BVs1NSuccpbI5fV4RgRdrFCn5s4ReylYipbLnySZnmyzGse4d3hzITsRSEuk2RQ%2BYTlH0mOwXzBhlXA5lLFPPyAVnjwetq42WLBNzlBKqyPQP5jHKkBZr5nhSrVNESZLJTiQ1g%2BWGU1dV5XtjLvC2JtKqu8MsxYLv5BQ9CjWKO7P7XONEoGWrGh%2FCciLSPEwOO1dQyYZG6weQ8x3kJpEgT9hBz8TiBHCIR1qTfqA0SygtURn4cCl%2FInCAxhgBytTPxrolsqpfeyoA%2BqkO4gpYIq14ogcEW3fDA883ieCB0GFC4DUwAY77YgJ8t%2BGW2IUWdkHgYAfDJuxAX9iFDnZ%2FLpfvJtyrCR9yC00D6aUdGviggQajvlgwdFmQvZPgoiSAIXxlEviuL3A4gGOZnOou42LFEpYheltJa15b6bOa85kpze258R0LsdNooI1gJnPwlohvavl1qHv3ejPV%2Fritd3a6cxSPnG14hF94aV0MCMQTLE5nO%2Br9X0SXY4r2WZBRBzRApZf%2BxYg8csWKwGRFGJo7FOfUiyy8D6c4nwLjFgygVJY%2B2MSsDO65PJ9YCCSwa%2FMHGztu851E%2FK49QEy4rOsIU0JJJ2WbneQCVkIfuAk9bMwEerL%2Bm9ew9fPttvSeJw037MVwJ5yjXW3CWllkftyuoeXtvRGwECt27NSay2e%2Bm3P%2F5mzX517o5nQXtWfPvVl5UOddCLxEMrkDd5ykskyTu4PfZh%2FkX%2FkLiz4lUl25RM4HCkGu7GwlC3swU6PT%2B6vFRH4G10CtJUqxmbJBJVwJoS7GJurw%2FjzieG9UEu6UZfk144mU6u2lucwfdlc5kp%2F7veYfHHpK7QuLmwbdSpLUuKlFJfAUL0VDHpeSOKbHilDTr9WodtPLXU44cn1%2F2JT6%2Bb2Rxb3M%2BcJJkmBeYDmkSocPkgjDRLVUdeDIGurG3uB7YEKwtBV8HeBlJWVBQ6z2mvDyvN4Ac%2B9cGiq2i2brfj1dL4ea03WJBN%2FVV6n%2BfX2wWrfv%2FXyeH7ZMF4Yt0wVNhUOM%2Fcm8PxiHFsUCc4viBZ3Ev2Gjoelb%2FHYVxPGExn1UOYctlznupQ7xApfeDe7lEvyuqGow1euNqcOOE9uOmeoPrcurkUWwtky1N4LwTTLVvTp7cnn6UpKNs%2Fj8FLuL4GZFN1hmjq%2BWuY4clf5Kln8qRpUR0auFQ3A9Cl%2BMiP0HNu%2BVIlto0cu3c6K2%2FmJoOx7POsvb8Bdtrth%2B3bDWTG6%2FL3K3jYWvRm7rCz2nQmtN7pG90Vskt%2B9eOL21YHhjleaeW%2BpdNBj67jWOo9EOvYER6CrXcMIbmF%2FVnKj9Tjq3892F39JdlDy9tLuAdso7Hp%2FnLgJgloswHLZyF904A9mt%2FmusmF797x28%2FR8%3D%3C%2Fdiagram%3E%3C%2Fmxfile%3E")
      {
        using namespace SubstatemachineEventTestImpl;
        SmBeginEnd sm;
        sm.begin();

        reset();

        sm.end();
        EQ(0, Active::entryCalls);
        EQ(1, Active::exitCalls);
        EQ(0, Active::doitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::entryCalls);
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::doitCalls);

        EQ(-1, OnState::lastEntryEvent);
        EQ(-1, OnState::lastExitEvent);
        EQ(-1, OnState::lastDoitEvent);
        EQ(-1, OffState::lastEntryEvent);
        EQ(000, OffState::lastExitEvent);
        EQ(-1, OffState::lastDoitEvent);
      }

      TEST(
        SubstateHaveEntryExitDoit,
        Dispatch,
        CallsMethodsWithDispatchedEvent,
        "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&page-id=vcinDnvGSfqwhQla_7vM&title=SubstatemachineTest.drawio#R%3Cmxfile%20pages%3D%222%22%3E%3Cdiagram%20id%3D%22YKBjTKGMOBF2p5uhFZQw%22%20name%3D%22Statemachine%22%3E7Vtdc9o4FP01mdk%2BJONv4BFokrLTbrIls%2F14yQhb2G5li5FFgPz6lbAEtmTAITahM0lngnUtX9v3nHt1dGku7GGyvCVgFn3BAUQXlhEsL%2ByPF5ZlGobBPrhllVu8Xjc3hCQOxKStYRw%2FQ3mlsM7jAGaliRRjRONZ2ejjNIU%2BLdkAIXhRnjbFqHzXGQihZhj7AOnWb3FAo9zadY2t%2FROMw0je2ZQvnAA5WRiyCAR4UTDZ1xf2kGBM86NkOYSIB0%2FGJb%2FuZsfZzYMRmNI6F9DF%2FQNEK3rdHy2W6fXz%2FdN191J4eQJoLl44S8Tj0pWMAXvyGT%2BcJ%2BiGgIQdDhZRTOF4BnxuXzDomS2iCWIjkx0SPE8DGIjR5s0NNvBxEvviGIEJRAPg%2Fw7XFwwxwoSdSnHK75FRgn9vos4dTXFKb0ASI06m%2FyAJQAqEWTDHZGEdABSHKRv4LDKQORzooZLvDQmFy4JJhO4W4gRSsmJTxFlboLgqDxcFTjjCFhX44MqJQPAw3HjeQsUOBFovQM7SkOv7NH6CGnplLA4AB4gvImk5PLIxQhKVC8uesh%2Ff2EBTOmPwVD8a65rI8nHhroYh7qohzoGNWRb3xQmKZ83wwHHKRDANV2OCY1Ywwe42wIT5l7v%2B6FMn8%2F52LwfeOBxPw1%2BX9nsO18Suo2DnOBp2tluFndFWFuvYjQL0nsPt5rCpFHPT0qu5VZnDXls8cDQe3E2n7zRolQauQgNm0WlgVNCg0xYLXJ0F6TsJTkoC27XfmARdDXEYsC2JGGJCIxziFKDrrbWwVvPobed8xjxOayb8gpSuROzBnOIyT1gAyeo7v%2F7KlcMfwt168HFZGq3EaCcCGZ4TH%2B55zV4%2BjwISQnp4heQx2IsngQis9W9pB9g0OD0tQ3W0EGKbU1iOr5RfGXtfOqaAQj0bN%2BzfnY2NaLKmczOICdt5x5gbGew8a9pYpR19kbYrtVpL0EtBcNrEPD7BTNkuOZRhTtMZtr60TwhYFSbMcJzSrOD5nhu2cHuqKOsYCmS5xy2Am0d7BaZ6%2F%2BM9n1vKZ9dVAHZ1udVWQtv%2FBtk%2F4aj%2Fk949fx49Th9%2Bfh9W9L4m%2FHHHFE4Bk13GLYkTtpFmzo2%2Fhh%2FYb%2FbPzscoZtHKGHCWwQEkPM0itm0zhvzs4MfluM8%2BnSuDXxvzuKY8BbkxopS3Lvv82a0bn8B1TjG0E5xmV5iEzCrcs2y5mawuM8A%2B175uPmjsZMGnCjVLbJMcKVBTmCTuCE5phcJK4iBAu9oE5bJWYFqvlW6b29Frv1slyqwGuPJ49zjKrEn0PPv6cxRFpP9r6FVw5YHEYQhJDqWHeAgnjAdeyI%2FGvCxUmVnV16xc42u2it1fa1BPMKU4qQV1A9gqDTSnYl03q7A1zQbArWyg6eBWbLtOIcKXMc01uOWK4Y%2FCqa0E5wOpwAvK3TqRdHdqCgu3prAQTNgorJcJDU1JdGVLdsMwp%2Bwif0FxVfEbE82RVy5DskEgHeUR0Bztlj76reQcPJ1msOTlCI1TyW79i4Kq4vKGe0yzNaK6DSvghonqdpUGVEfhV12iqo5s%2B08kqt4DzdfLnf2v1%2FLQLJXLQvncQURZny%2BNK8OwChWaXds5UKXZ4B4yQQnXGju3HUy545lv1WR%2B3e5Kw8y3PIWwajetLvM1R6pI2MH8lnh9P8bRIJv0%2BwB1fy8i%2Bu1xFVfIiye9%2FO7bZMI0OH6L2YBi62hBNjXJ1tbOrTKi%2BpJ2TuvZIeEly4hZqiAdd28BaVSt7aPpydWaQi5LzeC6paCnrqam8izntghWoqB%2FIXxO1D4k1aqpbbVF7Qp9d1bUVv6%2FgdaeqE3tnuroT6T2W%2Bu7Tl19Z5TYu5%2B8bYq4utuXuiJOsMm4Mk2nvLOVGfJKwjtaCT5yQ%2BMZqiPv7GSd%2FmX9HybrHKWquKbeiTuprPMOr32tFYfateHqBa25027w9rH01EufrW4Zut3jKoGjNPNs9%2FwqQefMNFv7zD6et%2FaZ81aRbHbvWN6aCm%2B9erw9L8kmo1rSbOuvtlqry0eyt7zjeDvNVsHvffW7vmbjXd0SoyTBXqvZnJ4itY5kvO2qmk1x1GqlZsPtn9Lk07d%2FkGRf%2Fw8%3D%3C%2Fdiagram%3E%3Cdiagram%20id%3D%22vcinDnvGSfqwhQla_7vM%22%20name%3D%22Begin%20End%22%3E7Vpbb%2BI4FP41SDsPrZw4AfoITOlIO6vdFaOd6dPKTUzw1ImRY1rYX782cUhsh5JhEjqV2krFPr7EOd93bqYDOEu3dxytV3%2BwGNOBD%2BLtAH4c%2BL4HAJAfSrIrJMObcSFIOIn1pEqwIP%2FhcqWWbkiMc2OiYIwKsjaFEcsyHAlDhjhnz%2Ba0JaPmU9cowY5gESHqSr%2BSWKwK6TgElfwTJsmqfLJXvnCKyslakK9QzJ5rIng7gDPOmCha6XaGqVJeqZdi3fzI6OFgHGeizYLPBMHF3de%2Fp0%2FfPj2uJ7%2BPs3%2B%2FXOldnhDd6BfOU31csSt1IE%2B%2BVs1NSuccpbI5fV4RgRdrFCn5s4ReylYipbLnySZnmyzGse4d3hzITsRSEuk2RQ%2BYTlH0mOwXzBhlXA5lLFPPyAVnjwetq42WLBNzlBKqyPQP5jHKkBZr5nhSrVNESZLJTiQ1g%2BWGU1dV5XtjLvC2JtKqu8MsxYLv5BQ9CjWKO7P7XONEoGWrGh%2FCciLSPEwOO1dQyYZG6weQ8x3kJpEgT9hBz8TiBHCIR1qTfqA0SygtURn4cCl%2FInCAxhgBytTPxrolsqpfeyoA%2BqkO4gpYIq14ogcEW3fDA883ieCB0GFC4DUwAY77YgJ8t%2BGW2IUWdkHgYAfDJuxAX9iFDnZ%2FLpfvJtyrCR9yC00D6aUdGviggQajvlgwdFmQvZPgoiSAIXxlEviuL3A4gGOZnOou42LFEpYheltJa15b6bOa85kpze258R0LsdNooI1gJnPwlohvavl1qHv3ejPV%2Fritd3a6cxSPnG14hF94aV0MCMQTLE5nO%2Br9X0SXY4r2WZBRBzRApZf%2BxYg8csWKwGRFGJo7FOfUiyy8D6c4nwLjFgygVJY%2B2MSsDO65PJ9YCCSwa%2FMHGztu851E%2FK49QEy4rOsIU0JJJ2WbneQCVkIfuAk9bMwEerL%2Bm9ew9fPttvSeJw037MVwJ5yjXW3CWllkftyuoeXtvRGwECt27NSay2e%2Bm3P%2F5mzX517o5nQXtWfPvVl5UOddCLxEMrkDd5ykskyTu4PfZh%2FkX%2FkLiz4lUl25RM4HCkGu7GwlC3swU6PT%2B6vFRH4G10CtJUqxmbJBJVwJoS7GJurw%2FjzieG9UEu6UZfk144mU6u2lucwfdlc5kp%2F7veYfHHpK7QuLmwbdSpLUuKlFJfAUL0VDHpeSOKbHilDTr9WodtPLXU44cn1%2F2JT6%2Bb2Rxb3M%2BcJJkmBeYDmkSocPkgjDRLVUdeDIGurG3uB7YEKwtBV8HeBlJWVBQ6z2mvDyvN4Ac%2B9cGiq2i2brfj1dL4ea03WJBN%2FVV6n%2BfX2wWrfv%2FXyeH7ZMF4Yt0wVNhUOM%2Fcm8PxiHFsUCc4viBZ3Ev2Gjoelb%2FHYVxPGExn1UOYctlznupQ7xApfeDe7lEvyuqGow1euNqcOOE9uOmeoPrcurkUWwtky1N4LwTTLVvTp7cnn6UpKNs%2Fj8FLuL4GZFN1hmjq%2BWuY4clf5Kln8qRpUR0auFQ3A9Cl%2BMiP0HNu%2BVIlto0cu3c6K2%2FmJoOx7POsvb8Bdtrth%2B3bDWTG6%2FL3K3jYWvRm7rCz2nQmtN7pG90Vskt%2B9eOL21YHhjleaeW%2BpdNBj67jWOo9EOvYER6CrXcMIbmF%2FVnKj9Tjq3892F39JdlDy9tLuAdso7Hp%2FnLgJgloswHLZyF904A9mt%2FmusmF797x28%2FR8%3D%3C%2Fdiagram%3E%3C%2Fmxfile%3E")
      {
        using namespace SubstatemachineEventTestImpl;
        SmBeginEnd sm;
        sm.begin();

        reset();

        sm.dispatch(Event::On{});
        EQ(0, Active::entryCalls);
        EQ(0, Active::exitCalls);
        EQ(0, Active::doitCalls);
        EQ(1, OnState::entryCalls);
        EQ(0, OnState::exitCalls);
        EQ(1, OnState::doitCalls);
        EQ(0, OffState::entryCalls);
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::doitCalls);

        EQ(100, OnState::lastEntryEvent);
        EQ(-1, OnState::lastExitEvent);
        EQ(100, OnState::lastDoitEvent);
        EQ(-1, OffState::lastEntryEvent);
        EQ(100, OffState::lastExitEvent);
        EQ(-1, OffState::lastDoitEvent);
      }

      TEST(
        ToStateHasSubstate,
        Dispatch,
        CallsEntryAndEntryAndDoitOnStateAndSubstate,
        "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=SubstatemachineTest.drawio#R%3Cmxfile%20pages%3D%222%22%3E%3Cdiagram%20id%3D%22YKBjTKGMOBF2p5uhFZQw%22%20name%3D%22Statemachine%22%3E7Vtdc9o4FP01mdk%2BJONv4BFokrLTbrIls%2F14yQhb2G5li5FFgPz6lbAEtmTAITahM0lngnUtX9v3nHt1dGku7GGyvCVgFn3BAUQXlhEsL%2ByPF5ZlGobBPrhllVu8Xjc3hCQOxKStYRw%2FQ3mlsM7jAGaliRRjRONZ2ejjNIU%2BLdkAIXhRnjbFqHzXGQihZhj7AOnWb3FAo9zadY2t%2FROMw0je2ZQvnAA5WRiyCAR4UTDZ1xf2kGBM86NkOYSIB0%2FGJb%2FuZsfZzYMRmNI6F9DF%2FQNEK3rdHy2W6fXz%2FdN191J4eQJoLl44S8Tj0pWMAXvyGT%2BcJ%2BiGgIQdDhZRTOF4BnxuXzDomS2iCWIjkx0SPE8DGIjR5s0NNvBxEvviGIEJRAPg%2Fw7XFwwxwoSdSnHK75FRgn9vos4dTXFKb0ASI06m%2FyAJQAqEWTDHZGEdABSHKRv4LDKQORzooZLvDQmFy4JJhO4W4gRSsmJTxFlboLgqDxcFTjjCFhX44MqJQPAw3HjeQsUOBFovQM7SkOv7NH6CGnplLA4AB4gvImk5PLIxQhKVC8uesh%2Ff2EBTOmPwVD8a65rI8nHhroYh7qohzoGNWRb3xQmKZ83wwHHKRDANV2OCY1Ywwe42wIT5l7v%2B6FMn8%2F52LwfeOBxPw1%2BX9nsO18Suo2DnOBp2tluFndFWFuvYjQL0nsPt5rCpFHPT0qu5VZnDXls8cDQe3E2n7zRolQauQgNm0WlgVNCg0xYLXJ0F6TsJTkoC27XfmARdDXEYsC2JGGJCIxziFKDrrbWwVvPobed8xjxOayb8gpSuROzBnOIyT1gAyeo7v%2F7KlcMfwt168HFZGq3EaCcCGZ4TH%2B55zV4%2BjwISQnp4heQx2IsngQis9W9pB9g0OD0tQ3W0EGKbU1iOr5RfGXtfOqaAQj0bN%2BzfnY2NaLKmczOICdt5x5gbGew8a9pYpR19kbYrtVpL0EtBcNrEPD7BTNkuOZRhTtMZtr60TwhYFSbMcJzSrOD5nhu2cHuqKOsYCmS5xy2Am0d7BaZ6%2F%2BM9n1vKZ9dVAHZ1udVWQtv%2FBtk%2F4aj%2Fk949fx49Th9%2Bfh9W9L4m%2FHHHFE4Bk13GLYkTtpFmzo2%2Fhh%2FYb%2FbPzscoZtHKGHCWwQEkPM0itm0zhvzs4MfluM8%2BnSuDXxvzuKY8BbkxopS3Lvv82a0bn8B1TjG0E5xmV5iEzCrcs2y5mawuM8A%2B175uPmjsZMGnCjVLbJMcKVBTmCTuCE5phcJK4iBAu9oE5bJWYFqvlW6b29Frv1slyqwGuPJ49zjKrEn0PPv6cxRFpP9r6FVw5YHEYQhJDqWHeAgnjAdeyI%2FGvCxUmVnV16xc42u2it1fa1BPMKU4qQV1A9gqDTSnYl03q7A1zQbArWyg6eBWbLtOIcKXMc01uOWK4Y%2FCqa0E5wOpwAvK3TqRdHdqCgu3prAQTNgorJcJDU1JdGVLdsMwp%2Bwif0FxVfEbE82RVy5DskEgHeUR0Bztlj76reQcPJ1msOTlCI1TyW79i4Kq4vKGe0yzNaK6DSvghonqdpUGVEfhV12iqo5s%2B08kqt4DzdfLnf2v1%2FLQLJXLQvncQURZny%2BNK8OwChWaXds5UKXZ4B4yQQnXGju3HUy545lv1WR%2B3e5Kw8y3PIWwajetLvM1R6pI2MH8lnh9P8bRIJv0%2BwB1fy8i%2Bu1xFVfIiye9%2FO7bZMI0OH6L2YBi62hBNjXJ1tbOrTKi%2BpJ2TuvZIeEly4hZqiAdd28BaVSt7aPpydWaQi5LzeC6paCnrqam8izntghWoqB%2FIXxO1D4k1aqpbbVF7Qp9d1bUVv6%2FgdaeqE3tnuroT6T2W%2Bu7Tl19Z5TYu5%2B8bYq4utuXuiJOsMm4Mk2nvLOVGfJKwjtaCT5yQ%2BMZqiPv7GSd%2FmX9HybrHKWquKbeiTuprPMOr32tFYfateHqBa25027w9rH01EufrW4Zut3jKoGjNPNs9%2FwqQefMNFv7zD6et%2FaZ81aRbHbvWN6aCm%2B9erw9L8kmo1rSbOuvtlqry0eyt7zjeDvNVsHvffW7vmbjXd0SoyTBXqvZnJ4itY5kvO2qmk1x1GqlZsPtn9Lk07d%2FkGRf%2Fw8%3D%3C%2Fdiagram%3E%3Cdiagram%20id%3D%22vcinDnvGSfqwhQla_7vM%22%20name%3D%22Begin%20End%22%3E7Vpbb%2BI4FP41SDsPrZw4AfoITOlIO6vdFaOd6dPKTUzw1ImRY1rYX782cUhsh5JhEjqV2krFPr7EOd93bqYDOEu3dxytV3%2BwGNOBD%2BLtAH4c%2BL4HAJAfSrIrJMObcSFIOIn1pEqwIP%2FhcqWWbkiMc2OiYIwKsjaFEcsyHAlDhjhnz%2Ba0JaPmU9cowY5gESHqSr%2BSWKwK6TgElfwTJsmqfLJXvnCKyslakK9QzJ5rIng7gDPOmCha6XaGqVJeqZdi3fzI6OFgHGeizYLPBMHF3de%2Fp0%2FfPj2uJ7%2BPs3%2B%2FXOldnhDd6BfOU31csSt1IE%2B%2BVs1NSuccpbI5fV4RgRdrFCn5s4ReylYipbLnySZnmyzGse4d3hzITsRSEuk2RQ%2BYTlH0mOwXzBhlXA5lLFPPyAVnjwetq42WLBNzlBKqyPQP5jHKkBZr5nhSrVNESZLJTiQ1g%2BWGU1dV5XtjLvC2JtKqu8MsxYLv5BQ9CjWKO7P7XONEoGWrGh%2FCciLSPEwOO1dQyYZG6weQ8x3kJpEgT9hBz8TiBHCIR1qTfqA0SygtURn4cCl%2FInCAxhgBytTPxrolsqpfeyoA%2BqkO4gpYIq14ogcEW3fDA883ieCB0GFC4DUwAY77YgJ8t%2BGW2IUWdkHgYAfDJuxAX9iFDnZ%2FLpfvJtyrCR9yC00D6aUdGviggQajvlgwdFmQvZPgoiSAIXxlEviuL3A4gGOZnOou42LFEpYheltJa15b6bOa85kpze258R0LsdNooI1gJnPwlohvavl1qHv3ejPV%2Fritd3a6cxSPnG14hF94aV0MCMQTLE5nO%2Br9X0SXY4r2WZBRBzRApZf%2BxYg8csWKwGRFGJo7FOfUiyy8D6c4nwLjFgygVJY%2B2MSsDO65PJ9YCCSwa%2FMHGztu851E%2FK49QEy4rOsIU0JJJ2WbneQCVkIfuAk9bMwEerL%2Bm9ew9fPttvSeJw037MVwJ5yjXW3CWllkftyuoeXtvRGwECt27NSay2e%2Bm3P%2F5mzX517o5nQXtWfPvVl5UOddCLxEMrkDd5ykskyTu4PfZh%2FkX%2FkLiz4lUl25RM4HCkGu7GwlC3swU6PT%2B6vFRH4G10CtJUqxmbJBJVwJoS7GJurw%2FjzieG9UEu6UZfk144mU6u2lucwfdlc5kp%2F7veYfHHpK7QuLmwbdSpLUuKlFJfAUL0VDHpeSOKbHilDTr9WodtPLXU44cn1%2F2JT6%2Bb2Rxb3M%2BcJJkmBeYDmkSocPkgjDRLVUdeDIGurG3uB7YEKwtBV8HeBlJWVBQ6z2mvDyvN4Ac%2B9cGiq2i2brfj1dL4ea03WJBN%2FVV6n%2BfX2wWrfv%2FXyeH7ZMF4Yt0wVNhUOM%2Fcm8PxiHFsUCc4viBZ3Ev2Gjoelb%2FHYVxPGExn1UOYctlznupQ7xApfeDe7lEvyuqGow1euNqcOOE9uOmeoPrcurkUWwtky1N4LwTTLVvTp7cnn6UpKNs%2Fj8FLuL4GZFN1hmjq%2BWuY4clf5Kln8qRpUR0auFQ3A9Cl%2BMiP0HNu%2BVIlto0cu3c6K2%2FmJoOx7POsvb8Bdtrth%2B3bDWTG6%2FL3K3jYWvRm7rCz2nQmtN7pG90Vskt%2B9eOL21YHhjleaeW%2BpdNBj67jWOo9EOvYER6CrXcMIbmF%2FVnKj9Tjq3892F39JdlDy9tLuAdso7Hp%2FnLgJgloswHLZyF904A9mt%2FmusmF797x28%2FR8%3D%3C%2Fdiagram%3E%3C%2Fmxfile%3E")
      {
        using namespace SubstatemachineEventTestImpl;
        Sm sm;
        sm.begin();

        reset();

        sm.dispatch(Event::Start{});

        EQ(0, Idle::entryCalls);
        EQ(1, Idle::exitCalls);
        EQ(0, Idle::doitCalls);
        EQ(1, Active::entryCalls);
        EQ(0, Active::exitCalls);
        EQ(0, Active::doitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::doitCalls);
        EQ(1, OffState::entryCalls);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::doitCalls);
      }

      TEST(
        FromStateWithSubstate,
        Dispatch,
        CallsExitAndExitOnSubstate,
        "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=SubstatemachineTest.drawio#R%3Cmxfile%20pages%3D%222%22%3E%3Cdiagram%20id%3D%22YKBjTKGMOBF2p5uhFZQw%22%20name%3D%22Statemachine%22%3E7Vtdc9o4FP01mdk%2BJONv4BFokrLTbrIls%2F14yQhb2G5li5FFgPz6lbAEtmTAITahM0lngnUtX9v3nHt1dGku7GGyvCVgFn3BAUQXlhEsL%2ByPF5ZlGobBPrhllVu8Xjc3hCQOxKStYRw%2FQ3mlsM7jAGaliRRjRONZ2ejjNIU%2BLdkAIXhRnjbFqHzXGQihZhj7AOnWb3FAo9zadY2t%2FROMw0je2ZQvnAA5WRiyCAR4UTDZ1xf2kGBM86NkOYSIB0%2FGJb%2FuZsfZzYMRmNI6F9DF%2FQNEK3rdHy2W6fXz%2FdN191J4eQJoLl44S8Tj0pWMAXvyGT%2BcJ%2BiGgIQdDhZRTOF4BnxuXzDomS2iCWIjkx0SPE8DGIjR5s0NNvBxEvviGIEJRAPg%2Fw7XFwwxwoSdSnHK75FRgn9vos4dTXFKb0ASI06m%2FyAJQAqEWTDHZGEdABSHKRv4LDKQORzooZLvDQmFy4JJhO4W4gRSsmJTxFlboLgqDxcFTjjCFhX44MqJQPAw3HjeQsUOBFovQM7SkOv7NH6CGnplLA4AB4gvImk5PLIxQhKVC8uesh%2Ff2EBTOmPwVD8a65rI8nHhroYh7qohzoGNWRb3xQmKZ83wwHHKRDANV2OCY1Ywwe42wIT5l7v%2B6FMn8%2F52LwfeOBxPw1%2BX9nsO18Suo2DnOBp2tluFndFWFuvYjQL0nsPt5rCpFHPT0qu5VZnDXls8cDQe3E2n7zRolQauQgNm0WlgVNCg0xYLXJ0F6TsJTkoC27XfmARdDXEYsC2JGGJCIxziFKDrrbWwVvPobed8xjxOayb8gpSuROzBnOIyT1gAyeo7v%2F7KlcMfwt168HFZGq3EaCcCGZ4TH%2B55zV4%2BjwISQnp4heQx2IsngQis9W9pB9g0OD0tQ3W0EGKbU1iOr5RfGXtfOqaAQj0bN%2BzfnY2NaLKmczOICdt5x5gbGew8a9pYpR19kbYrtVpL0EtBcNrEPD7BTNkuOZRhTtMZtr60TwhYFSbMcJzSrOD5nhu2cHuqKOsYCmS5xy2Am0d7BaZ6%2F%2BM9n1vKZ9dVAHZ1udVWQtv%2FBtk%2F4aj%2Fk949fx49Th9%2Bfh9W9L4m%2FHHHFE4Bk13GLYkTtpFmzo2%2Fhh%2FYb%2FbPzscoZtHKGHCWwQEkPM0itm0zhvzs4MfluM8%2BnSuDXxvzuKY8BbkxopS3Lvv82a0bn8B1TjG0E5xmV5iEzCrcs2y5mawuM8A%2B175uPmjsZMGnCjVLbJMcKVBTmCTuCE5phcJK4iBAu9oE5bJWYFqvlW6b29Frv1slyqwGuPJ49zjKrEn0PPv6cxRFpP9r6FVw5YHEYQhJDqWHeAgnjAdeyI%2FGvCxUmVnV16xc42u2it1fa1BPMKU4qQV1A9gqDTSnYl03q7A1zQbArWyg6eBWbLtOIcKXMc01uOWK4Y%2FCqa0E5wOpwAvK3TqRdHdqCgu3prAQTNgorJcJDU1JdGVLdsMwp%2Bwif0FxVfEbE82RVy5DskEgHeUR0Bztlj76reQcPJ1msOTlCI1TyW79i4Kq4vKGe0yzNaK6DSvghonqdpUGVEfhV12iqo5s%2B08kqt4DzdfLnf2v1%2FLQLJXLQvncQURZny%2BNK8OwChWaXds5UKXZ4B4yQQnXGju3HUy545lv1WR%2B3e5Kw8y3PIWwajetLvM1R6pI2MH8lnh9P8bRIJv0%2BwB1fy8i%2Bu1xFVfIiye9%2FO7bZMI0OH6L2YBi62hBNjXJ1tbOrTKi%2BpJ2TuvZIeEly4hZqiAdd28BaVSt7aPpydWaQi5LzeC6paCnrqam8izntghWoqB%2FIXxO1D4k1aqpbbVF7Qp9d1bUVv6%2FgdaeqE3tnuroT6T2W%2Bu7Tl19Z5TYu5%2B8bYq4utuXuiJOsMm4Mk2nvLOVGfJKwjtaCT5yQ%2BMZqiPv7GSd%2FmX9HybrHKWquKbeiTuprPMOr32tFYfateHqBa25027w9rH01EufrW4Zut3jKoGjNPNs9%2FwqQefMNFv7zD6et%2FaZ81aRbHbvWN6aCm%2B9erw9L8kmo1rSbOuvtlqry0eyt7zjeDvNVsHvffW7vmbjXd0SoyTBXqvZnJ4itY5kvO2qmk1x1GqlZsPtn9Lk07d%2FkGRf%2Fw8%3D%3C%2Fdiagram%3E%3Cdiagram%20id%3D%22vcinDnvGSfqwhQla_7vM%22%20name%3D%22Begin%20End%22%3E7Vpbb%2BI4FP41SDsPrZw4AfoITOlIO6vdFaOd6dPKTUzw1ImRY1rYX782cUhsh5JhEjqV2krFPr7EOd93bqYDOEu3dxytV3%2BwGNOBD%2BLtAH4c%2BL4HAJAfSrIrJMObcSFIOIn1pEqwIP%2FhcqWWbkiMc2OiYIwKsjaFEcsyHAlDhjhnz%2Ba0JaPmU9cowY5gESHqSr%2BSWKwK6TgElfwTJsmqfLJXvnCKyslakK9QzJ5rIng7gDPOmCha6XaGqVJeqZdi3fzI6OFgHGeizYLPBMHF3de%2Fp0%2FfPj2uJ7%2BPs3%2B%2FXOldnhDd6BfOU31csSt1IE%2B%2BVs1NSuccpbI5fV4RgRdrFCn5s4ReylYipbLnySZnmyzGse4d3hzITsRSEuk2RQ%2BYTlH0mOwXzBhlXA5lLFPPyAVnjwetq42WLBNzlBKqyPQP5jHKkBZr5nhSrVNESZLJTiQ1g%2BWGU1dV5XtjLvC2JtKqu8MsxYLv5BQ9CjWKO7P7XONEoGWrGh%2FCciLSPEwOO1dQyYZG6weQ8x3kJpEgT9hBz8TiBHCIR1qTfqA0SygtURn4cCl%2FInCAxhgBytTPxrolsqpfeyoA%2BqkO4gpYIq14ogcEW3fDA883ieCB0GFC4DUwAY77YgJ8t%2BGW2IUWdkHgYAfDJuxAX9iFDnZ%2FLpfvJtyrCR9yC00D6aUdGviggQajvlgwdFmQvZPgoiSAIXxlEviuL3A4gGOZnOou42LFEpYheltJa15b6bOa85kpze258R0LsdNooI1gJnPwlohvavl1qHv3ejPV%2Fritd3a6cxSPnG14hF94aV0MCMQTLE5nO%2Br9X0SXY4r2WZBRBzRApZf%2BxYg8csWKwGRFGJo7FOfUiyy8D6c4nwLjFgygVJY%2B2MSsDO65PJ9YCCSwa%2FMHGztu851E%2FK49QEy4rOsIU0JJJ2WbneQCVkIfuAk9bMwEerL%2Bm9ew9fPttvSeJw037MVwJ5yjXW3CWllkftyuoeXtvRGwECt27NSay2e%2Bm3P%2F5mzX517o5nQXtWfPvVl5UOddCLxEMrkDd5ykskyTu4PfZh%2FkX%2FkLiz4lUl25RM4HCkGu7GwlC3swU6PT%2B6vFRH4G10CtJUqxmbJBJVwJoS7GJurw%2FjzieG9UEu6UZfk144mU6u2lucwfdlc5kp%2F7veYfHHpK7QuLmwbdSpLUuKlFJfAUL0VDHpeSOKbHilDTr9WodtPLXU44cn1%2F2JT6%2Bb2Rxb3M%2BcJJkmBeYDmkSocPkgjDRLVUdeDIGurG3uB7YEKwtBV8HeBlJWVBQ6z2mvDyvN4Ac%2B9cGiq2i2brfj1dL4ea03WJBN%2FVV6n%2BfX2wWrfv%2FXyeH7ZMF4Yt0wVNhUOM%2Fcm8PxiHFsUCc4viBZ3Ev2Gjoelb%2FHYVxPGExn1UOYctlznupQ7xApfeDe7lEvyuqGow1euNqcOOE9uOmeoPrcurkUWwtky1N4LwTTLVvTp7cnn6UpKNs%2Fj8FLuL4GZFN1hmjq%2BWuY4clf5Kln8qRpUR0auFQ3A9Cl%2BMiP0HNu%2BVIlto0cu3c6K2%2FmJoOx7POsvb8Bdtrth%2B3bDWTG6%2FL3K3jYWvRm7rCz2nQmtN7pG90Vskt%2B9eOL21YHhjleaeW%2BpdNBj67jWOo9EOvYER6CrXcMIbmF%2FVnKj9Tjq3892F39JdlDy9tLuAdso7Hp%2FnLgJgloswHLZyF904A9mt%2FmusmF797x28%2FR8%3D%3C%2Fdiagram%3E%3C%2Fmxfile%3E")
      {
        using namespace SubstatemachineEventTestImpl;
        Sm sm;
        sm.begin();
        sm.dispatch(Event::Start{});

        reset();

        sm.dispatch(Event::Stop{});
        EQ(1, Idle::entryCalls);
        EQ(0, Idle::exitCalls);
        EQ(1, Idle::doitCalls);
        EQ(0, Active::entryCalls);
        EQ(1, Active::exitCalls);
        EQ(0, Active::doitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::entryCalls);
        EQ(1, OffState::exitCalls);
        EQ(0, OffState::doitCalls);
      }

      TEST(
        FromSubstate,
        Dispatch,
        CallsExitAndExitOnSubstate,
        "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=SubstatemachineTest.drawio#R%3Cmxfile%20pages%3D%222%22%3E%3Cdiagram%20id%3D%22YKBjTKGMOBF2p5uhFZQw%22%20name%3D%22Statemachine%22%3E7Vtdc9o4FP01mdk%2BJONv4BFokrLTbrIls%2F14yQhb2G5li5FFgPz6lbAEtmTAITahM0lngnUtX9v3nHt1dGku7GGyvCVgFn3BAUQXlhEsL%2ByPF5ZlGobBPrhllVu8Xjc3hCQOxKStYRw%2FQ3mlsM7jAGaliRRjRONZ2ejjNIU%2BLdkAIXhRnjbFqHzXGQihZhj7AOnWb3FAo9zadY2t%2FROMw0je2ZQvnAA5WRiyCAR4UTDZ1xf2kGBM86NkOYSIB0%2FGJb%2FuZsfZzYMRmNI6F9DF%2FQNEK3rdHy2W6fXz%2FdN191J4eQJoLl44S8Tj0pWMAXvyGT%2BcJ%2BiGgIQdDhZRTOF4BnxuXzDomS2iCWIjkx0SPE8DGIjR5s0NNvBxEvviGIEJRAPg%2Fw7XFwwxwoSdSnHK75FRgn9vos4dTXFKb0ASI06m%2FyAJQAqEWTDHZGEdABSHKRv4LDKQORzooZLvDQmFy4JJhO4W4gRSsmJTxFlboLgqDxcFTjjCFhX44MqJQPAw3HjeQsUOBFovQM7SkOv7NH6CGnplLA4AB4gvImk5PLIxQhKVC8uesh%2Ff2EBTOmPwVD8a65rI8nHhroYh7qohzoGNWRb3xQmKZ83wwHHKRDANV2OCY1Ywwe42wIT5l7v%2B6FMn8%2F52LwfeOBxPw1%2BX9nsO18Suo2DnOBp2tluFndFWFuvYjQL0nsPt5rCpFHPT0qu5VZnDXls8cDQe3E2n7zRolQauQgNm0WlgVNCg0xYLXJ0F6TsJTkoC27XfmARdDXEYsC2JGGJCIxziFKDrrbWwVvPobed8xjxOayb8gpSuROzBnOIyT1gAyeo7v%2F7KlcMfwt168HFZGq3EaCcCGZ4TH%2B55zV4%2BjwISQnp4heQx2IsngQis9W9pB9g0OD0tQ3W0EGKbU1iOr5RfGXtfOqaAQj0bN%2BzfnY2NaLKmczOICdt5x5gbGew8a9pYpR19kbYrtVpL0EtBcNrEPD7BTNkuOZRhTtMZtr60TwhYFSbMcJzSrOD5nhu2cHuqKOsYCmS5xy2Am0d7BaZ6%2F%2BM9n1vKZ9dVAHZ1udVWQtv%2FBtk%2F4aj%2Fk949fx49Th9%2Bfh9W9L4m%2FHHHFE4Bk13GLYkTtpFmzo2%2Fhh%2FYb%2FbPzscoZtHKGHCWwQEkPM0itm0zhvzs4MfluM8%2BnSuDXxvzuKY8BbkxopS3Lvv82a0bn8B1TjG0E5xmV5iEzCrcs2y5mawuM8A%2B175uPmjsZMGnCjVLbJMcKVBTmCTuCE5phcJK4iBAu9oE5bJWYFqvlW6b29Frv1slyqwGuPJ49zjKrEn0PPv6cxRFpP9r6FVw5YHEYQhJDqWHeAgnjAdeyI%2FGvCxUmVnV16xc42u2it1fa1BPMKU4qQV1A9gqDTSnYl03q7A1zQbArWyg6eBWbLtOIcKXMc01uOWK4Y%2FCqa0E5wOpwAvK3TqRdHdqCgu3prAQTNgorJcJDU1JdGVLdsMwp%2Bwif0FxVfEbE82RVy5DskEgHeUR0Bztlj76reQcPJ1msOTlCI1TyW79i4Kq4vKGe0yzNaK6DSvghonqdpUGVEfhV12iqo5s%2B08kqt4DzdfLnf2v1%2FLQLJXLQvncQURZny%2BNK8OwChWaXds5UKXZ4B4yQQnXGju3HUy545lv1WR%2B3e5Kw8y3PIWwajetLvM1R6pI2MH8lnh9P8bRIJv0%2BwB1fy8i%2Bu1xFVfIiye9%2FO7bZMI0OH6L2YBi62hBNjXJ1tbOrTKi%2BpJ2TuvZIeEly4hZqiAdd28BaVSt7aPpydWaQi5LzeC6paCnrqam8izntghWoqB%2FIXxO1D4k1aqpbbVF7Qp9d1bUVv6%2FgdaeqE3tnuroT6T2W%2Bu7Tl19Z5TYu5%2B8bYq4utuXuiJOsMm4Mk2nvLOVGfJKwjtaCT5yQ%2BMZqiPv7GSd%2FmX9HybrHKWquKbeiTuprPMOr32tFYfateHqBa25027w9rH01EufrW4Zut3jKoGjNPNs9%2FwqQefMNFv7zD6et%2FaZ81aRbHbvWN6aCm%2B9erw9L8kmo1rSbOuvtlqry0eyt7zjeDvNVsHvffW7vmbjXd0SoyTBXqvZnJ4itY5kvO2qmk1x1GqlZsPtn9Lk07d%2FkGRf%2Fw8%3D%3C%2Fdiagram%3E%3Cdiagram%20id%3D%22vcinDnvGSfqwhQla_7vM%22%20name%3D%22Begin%20End%22%3E7Vpbb%2BI4FP41SDsPrZw4AfoITOlIO6vdFaOd6dPKTUzw1ImRY1rYX782cUhsh5JhEjqV2krFPr7EOd93bqYDOEu3dxytV3%2BwGNOBD%2BLtAH4c%2BL4HAJAfSrIrJMObcSFIOIn1pEqwIP%2FhcqWWbkiMc2OiYIwKsjaFEcsyHAlDhjhnz%2Ba0JaPmU9cowY5gESHqSr%2BSWKwK6TgElfwTJsmqfLJXvnCKyslakK9QzJ5rIng7gDPOmCha6XaGqVJeqZdi3fzI6OFgHGeizYLPBMHF3de%2Fp0%2FfPj2uJ7%2BPs3%2B%2FXOldnhDd6BfOU31csSt1IE%2B%2BVs1NSuccpbI5fV4RgRdrFCn5s4ReylYipbLnySZnmyzGse4d3hzITsRSEuk2RQ%2BYTlH0mOwXzBhlXA5lLFPPyAVnjwetq42WLBNzlBKqyPQP5jHKkBZr5nhSrVNESZLJTiQ1g%2BWGU1dV5XtjLvC2JtKqu8MsxYLv5BQ9CjWKO7P7XONEoGWrGh%2FCciLSPEwOO1dQyYZG6weQ8x3kJpEgT9hBz8TiBHCIR1qTfqA0SygtURn4cCl%2FInCAxhgBytTPxrolsqpfeyoA%2BqkO4gpYIq14ogcEW3fDA883ieCB0GFC4DUwAY77YgJ8t%2BGW2IUWdkHgYAfDJuxAX9iFDnZ%2FLpfvJtyrCR9yC00D6aUdGviggQajvlgwdFmQvZPgoiSAIXxlEviuL3A4gGOZnOou42LFEpYheltJa15b6bOa85kpze258R0LsdNooI1gJnPwlohvavl1qHv3ejPV%2Fritd3a6cxSPnG14hF94aV0MCMQTLE5nO%2Br9X0SXY4r2WZBRBzRApZf%2BxYg8csWKwGRFGJo7FOfUiyy8D6c4nwLjFgygVJY%2B2MSsDO65PJ9YCCSwa%2FMHGztu851E%2FK49QEy4rOsIU0JJJ2WbneQCVkIfuAk9bMwEerL%2Bm9ew9fPttvSeJw037MVwJ5yjXW3CWllkftyuoeXtvRGwECt27NSay2e%2Bm3P%2F5mzX517o5nQXtWfPvVl5UOddCLxEMrkDd5ykskyTu4PfZh%2FkX%2FkLiz4lUl25RM4HCkGu7GwlC3swU6PT%2B6vFRH4G10CtJUqxmbJBJVwJoS7GJurw%2FjzieG9UEu6UZfk144mU6u2lucwfdlc5kp%2F7veYfHHpK7QuLmwbdSpLUuKlFJfAUL0VDHpeSOKbHilDTr9WodtPLXU44cn1%2F2JT6%2Bb2Rxb3M%2BcJJkmBeYDmkSocPkgjDRLVUdeDIGurG3uB7YEKwtBV8HeBlJWVBQ6z2mvDyvN4Ac%2B9cGiq2i2brfj1dL4ea03WJBN%2FVV6n%2BfX2wWrfv%2FXyeH7ZMF4Yt0wVNhUOM%2Fcm8PxiHFsUCc4viBZ3Ev2Gjoelb%2FHYVxPGExn1UOYctlznupQ7xApfeDe7lEvyuqGow1euNqcOOE9uOmeoPrcurkUWwtky1N4LwTTLVvTp7cnn6UpKNs%2Fj8FLuL4GZFN1hmjq%2BWuY4clf5Kln8qRpUR0auFQ3A9Cl%2BMiP0HNu%2BVIlto0cu3c6K2%2FmJoOx7POsvb8Bdtrth%2B3bDWTG6%2FL3K3jYWvRm7rCz2nQmtN7pG90Vskt%2B9eOL21YHhjleaeW%2BpdNBj67jWOo9EOvYER6CrXcMIbmF%2FVnKj9Tjq3892F39JdlDy9tLuAdso7Hp%2FnLgJgloswHLZyF904A9mt%2FmusmF797x28%2FR8%3D%3C%2Fdiagram%3E%3C%2Fmxfile%3E")
      {
        using namespace SubstatemachineEventTestImpl;
        Sm sm;
        sm.begin();
        sm.dispatch(Event::Start{});
        sm.dispatch(Event::On{});

        reset();

        sm.dispatch(Event::Stop{});
        EQ(1, Idle::entryCalls);
        EQ(0, Idle::exitCalls);
        EQ(1, Idle::doitCalls);
        EQ(0, Active::entryCalls);
        EQ(1, Active::exitCalls);
        EQ(0, Active::doitCalls);
        EQ(0, OnState::entryCalls);
        EQ(1, OnState::exitCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::exitCalls);
        EQ(0, OffState::doitCalls);
      }

      void reset() const
      {
        using namespace SubstatemachineEventTestImpl;
        Idle::entryCalls = 0;
        Idle::exitCalls = 0;
        Idle::doitCalls = 0;
        Active::entryCalls = 0;
        Active::exitCalls = 0;
        Active::doitCalls = 0;
        OnState::reset();
        OffState::reset();
      }

    END

  }
}
