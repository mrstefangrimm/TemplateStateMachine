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

    namespace DispatchSameEventSubstateTestImpl {

      using StatePolicy = State<VirtualTypeIdComparator, false>;
      using RecorderType = Recorder<sizeof(__FILE__) + __LINE__>;
      template<class Derived> struct Leaf : BasicState<Derived, StatePolicy, true, true, true>, FactoryCreatorFake<Derived> {};
      template<class Derived, class Statemachine> struct Composite : SubstatesHolderState<Derived, StatePolicy, Statemachine, true, true>, FactoryCreatorFake<Derived> {};

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
        struct Count {};
      }

      struct A : Leaf<A> {
        static const char* name;
        int counter = 0;

        template<class Event> void entry(const Event&) { RecorderType::add("A::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("A::Exit"); }
        template<class Event> void doit(const Event&) {
          RecorderType::add("A::Do");
          counter++;
        }
        uint8_t getTypeId() const override { return 1; };
      };
      const char* A::name = "A";

      struct BA : Leaf<BA> {
        static const char* name;
        int counter = 0;
        
        template<class Event> void entry(const Event&) { RecorderType::add("BA::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("BA::Exit"); }
        template<class Event> void doit(const Event&) {
          RecorderType::add("BA::Do");
          counter++;
        }

        uint8_t getTypeId() const override { return 3; };
      };
      const char* BA::name = "BA";

      struct GotoBCountGuardA {
        template<class StateType, class EventType>
        bool eval(const StateType& state, const EventType& ev) {
          return state.counter > 2;
        }
      };

      struct GotoACountGuardBA {
        template<class StateType, class EventType>
        bool eval(const StateType& state, const EventType& ev) {
          return state.counter > 2;
        }
      };

      using ToAorSelfBA = ChoiceExitTransition<Trigger::Count, A, BA, BA, GotoACountGuardBA, ActionChoiceRecordingSpy<BA>>;
      using ToFinalFromBA = FinalTransition<BA>;
      using BTransitions =
        Typelist<ToAorSelfBA,
        Typelist<ToFinalFromBA,
        NullType>>;

      using BInitTransition = InitialTransition<BA, ActionSpy<BA, InitialStateNamedFake<StatePolicy>, RecorderType>>;
      using BSm = Statemachine<BTransitions, BInitTransition>;

      struct B : Composite<B, BSm> {
        static const char* name;

        template<class Event> void entry(const Event&) { RecorderType::add("B::Entry"); }
        template<class Event> void exit(const Event&) { RecorderType::add("B::Exit"); }
        template<class Event> void doit(const Event&) {
          RecorderType::add("B::Do");
        }

        uint8_t getTypeId() const override { return 2; };
      };
      const char* B::name = "B";

      using ToBorSelfA = ChoiceTransition<Trigger::Count, B, A, A, GotoBCountGuardA, ActionChoiceRecordingSpy<A>>;
      using DeclToAorSelfBA = ExitDeclaration<Trigger::Count, A, B>;
      using Transitions =
        Typelist<ToBorSelfA,
        Typelist<DeclToAorSelfBA,
        NullType>>;

      using ToplevelInitTransition = InitialTransition<A, ActionSpy<A, InitialStateNamedFake<StatePolicy>, RecorderType>>;
      using Sm = Statemachine<Transitions, ToplevelInitTransition>;
    }

    BEGIN(
      DispatchSameEventSubstateTest,
      "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=DispatchSameEventSubstateTest.drawio#R7Vpdc5s4FP01ntk%2BOMO37cfYifvdyaw77XrfMMhAK5Ar5Njk169kBAZJxsQFp91pHlp0kS5w7zlXR5IH5izev8buJvyIfAAHhubvB%2BbdwDB0TdPof8yS5RZnMs4NAY583uloWERPoBjJrdvIB2mtI0EIkmhTN3ooSYBHajYXY7Srd1sjWH%2Fqxg2AZFh4LpStXyOfhLl1bGtH%2BxsQBWHxZL344NgtOnNDGro%2B2lVM5v3AnGGESH4V72cAsuAVccnHzU%2FcLV8Mg4S0GUB2D58BzMj97dvdPrl%2Feni8Hw%2B5l0cXbvkHpzF%2FXZIVMaBvvmGX2xjOsRvTy%2BkujAhYbFyP2Xc09dQWkhjSlk4vMdomPvB5q%2FxyjTY8FEcev4buCsCp630PDgNmCCJMbyUoYc9ICUbfy6gzR2uUkLkbR5CB6QvAvpu43MyRo9OwTl0YBQlteDQygDqcyqEqvhtgAvYVEw%2Fda4BiQHBGu%2FC7Js9iVm%2FujpiYFJgIK3iwLW50OQ6D0vMxVfSCZ0uduTffsv38Ybv5MXwAzsfddr9YQkXmplLi6mk4kzMXezyIhsWCGkFYJGRgmGv652llVmp3NMbyi9PcMqmsXXmqpvGnSslmOY0ogW%2F5DYI23UDAseoYGNsSBgxLgQFT6wADSvYaEgZu%2F2CgVwwYIgbkOnBlDJhyHfgDgp4LgV0Hge4oUGAoUFB27BwFYynlwKc6hjcRJiEKUOLC%2B6O1Mkmz8B37fEAsUAcofAOEZDz47pagOlBOhjJFW%2ByBhtedcCXn4gCQ8wWOfUtjYjCALoke65qt8yBPJKrJUYeQKlNQj1Ohn1L6vWRBXAJkWpUwPk2rTkRV1yTzI0xld4SYkaadwb8LipVM4RSzZIaZyjLbU%2Bp1S8r9jEaXNFTaC0lFY4azf6qNJXN2YxfNuz13nreyorWPSDmMXuejRjZvHkexRjHocgabMoPH8IP1xcqgZ3nh282PzP6EVq0ZzPM%2B1G50%2BlcvrzyprUnOvT%2BgiCWo7ILW6xQQCQrlS7RCR9NHNhUGHKJ4tU3Pz74dkGdk2bUA0gnrauxRxseW42PTmrU91BA2UTpuzMIQkEMANHr7rgdmcYZoNYY0E%2BQUG0fn6NiKWU1g6mxuPDLLcTSrBoyhbv8ctbhvayIIoomApPyj%2BajqHoXgyBaVlegoj0qDo%2F7JrpgKJKyeU9jpJt%2BrWkd7huhOSG8LpNdl0usqTdoB6SMwwsbyx3Kx%2BDq1n9689%2BL3d4oNihV73QUBazehF69xFMcDgzrX%2Fpq9YqzXDDNvw4jqi5RGxtAY51mNICGg%2F87Y3elyuKBLHM260djYiFWRhFGFGUNC2M7gLXt3Y%2B5hcMA11UcxStIbhANq5e5TernKhqlL%2Fz%2F4mr%2BSEkmDTwQxV9NnhaqqiDluKpQSBGuiWIzEke%2FDUztp9VJX0WaTXjazbMUq1tYUWDH6woo8gX7GURAAnKfSgSyEK4oD5zBFqKVXb7laIUJQ3CpXHSRH2GGwbDk5uio5ehdM%2FuRNYRZOlp%2F%2Ftd4h5K2fgkfVVuOLyhtTkDf6FRcHyvjI6O17bVDqjeetDerKx%2BhrbaBQME24eoaC0ayRU8%2F9T64Nim09aUfHfqaAOa07pIeZwkwtPeyEyLlAwCjDrtAvz1fjV9heOoV247KlcAdob0JxFe1NBzbPQLs9tiZ1pHQj101xc0WcOdrK9bHgZ%2Fyial2ZnbZLz4PCOHQ4gffEv2XHyUfRoDzurK0ZK4V5WcX0uSpdQnVZRWrHsHV6K9LSBs5QsRRRHpZ2U8zF1ShdGl0Gb1NwZGrtCjUFiptVum1Yh%2FSlGODIDPgFlqviJEizL2Gkr%2BXqev7OWjw5d98%2B7vfxUxjs4d%2FvW8%2BK%2F%2BdC0XY%2FqnFf5DeqFCOxUogTWNtKITkyfvFKoVQocmG4qsqrk0B7KRIoQ6M47mg6FunuwPLEPunEuhEWEfqFk9zIkVyJv1nqbj3SFNs%2Fx6r9H6s6I2Gdq19v60SZe3nWfXxe8ul0e3nqO4ioeNZmXnEzShlRxYLnt6%2Fq%2Bs9X9baH2E0oPVvVSxXTkTwRf3Q6Ely0rvEjEaSCoxeWJ7R5%2FB103v34a3Lz%2Fj8%3D")
   
      INIT(
        Initialize,
        {
          using namespace DispatchSameEventSubstateTestImpl;
          RecorderType::reset();
          FactoryCreatorFake<A>::reset();
          FactoryCreatorFake<B>::reset();
          FactoryCreatorFake<BA>::reset();
        })

      TEST(
        Count,
        CounterOverThreshold,
        B)
      {
        using namespace DispatchSameEventSubstateTestImpl;
        Sm sm;

        sm.begin();

        sm.dispatch<Trigger::Count>();
        RecorderType::reset();
        sm.dispatch<Trigger::Count>();
        RecorderType::check({
          "?<-A",
          "A::Do" });

        sm.dispatch<Trigger::Count>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "B::Entry",
          "BA<-Initial",
          "BA::Entry",
          "BA::Do" });

        // Active state is B/BA
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
        EQ(FactoryCreatorFake<BA>::createCalls, FactoryCreatorFake<BA>::deleteCalls + 1);
      }

      TEST(
        Count,
        CounterOverThreshold,
        BthenA)
      {
        using namespace DispatchSameEventSubstateTestImpl;
        Sm sm;

        sm.begin();
        sm.dispatch<Trigger::Count>();
        sm.dispatch<Trigger::Count>();
        RecorderType::reset();
        sm.dispatch<Trigger::Count>();
        RecorderType::check({
          "?<-A",
          "A::Exit",
          "B::Entry",
          "BA<-Initial",
          "BA::Entry",
          "BA::Do" });

        sm.dispatch<Trigger::Count>();
        RecorderType::check({
          "?<-BA",
          "BA::Do" });

        auto result = sm.dispatch<Trigger::Count>();
        RecorderType::reset();
        sm.dispatch<Trigger::Count>();
        RecorderType::check({
          "?<-BA",
          "BA::Exit",
          "B::Exit",
          "A::Entry",
          "A::Do" });

        // Active state is A
        EQ(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls + 1);
        EQ(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
        EQ(FactoryCreatorFake<BA>::createCalls, FactoryCreatorFake<BA>::deleteCalls);
      }

    END

  }
}
