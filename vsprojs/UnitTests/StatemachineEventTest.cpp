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

namespace UT {
  namespace Classes {

    using namespace tsmlib;

    namespace StatemachineEventTestImpl {

      namespace Event {
        struct On {};
        struct Off {};
        struct Self {};
        struct Reenter {};
      }

      using StatePolicy = State<VirtualTypeIdComparator, false>;

      struct OnState : BasicState<OnState, StatePolicy, true, true, true>, FactoryCreator<OnState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 1; }

      private:
        friend class BasicState<OnState, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int OnState::entryCalls = 0;
      int OnState::exitCalls = 0;
      int OnState::doitCalls = 0;

      struct OffState : BasicState<OffState, StatePolicy, true, true, true>, FactoryCreator<OffState> {
        static int entryCalls;
        static int exitCalls;
        static int doitCalls;

        uint8_t getTypeId() const override { return 2; }

      private:
        friend class BasicState<OffState, StatePolicy, true, true, true>;
        template<class Event> void entry(const Event&) { entryCalls++; }
        template<class Event> void exit(const Event&) { exitCalls++; }
        template<class Event> void doit(const Event&) { doitCalls++; }
      };
      int OffState::entryCalls = 0;
      int OffState::exitCalls = 0;
      int OffState::doitCalls = 0;

      using ToOnFromOffTransition = Transition<Event::On, OnState, OffState, NoGuard, NoAction>;
      using ToOffFromOnTransition = Transition<Event::Off, OffState, OnState, NoGuard, NoAction>;
      using ToOnFromOnSelfTransition = SelfTransition<Event::Self, OnState, NoGuard, NoAction, false>;
      using ToOffFromOffSelfTransition = SelfTransition<Event::Self, OffState, NoGuard, NoAction, false>;
      using ToOnFromOnReenterTransition = SelfTransition<Event::Reenter, OnState, NoGuard, NoAction, true>;
      using ToOffFromOffReenterTransition = SelfTransition<Event::Reenter, OffState, NoGuard, NoAction, true>;
      using ToFinalFromOnTransition = FinalTransition<OnState>;

      using TransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToOnFromOnSelfTransition,
        Typelist<ToOffFromOffSelfTransition,
        Typelist<ToOnFromOnReenterTransition,
        Typelist<ToOffFromOffReenterTransition,
        Typelist<ToFinalFromOnTransition,
        NullType>>>>>>>;

      using InitTransition = InitialTransition<OffState, NoAction>;
      using Sm = Statemachine<TransitionList, InitTransition>;
    }

    BEGIN(
      StatemachineEventTest,
      "https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=StateMachineTest.drawio#R7Vptc%2BI2EP41zPQ%2BkPErmI%2BQQC4zl2t65LikXzrCFliNbHFCCTi%2FvlIs4xcJ4xCTXKclM0FaS2t799lnVxId%2BzzaXlKwCq9JAHHHMoJtx77oWJZpGAb%2FEpIklfQGXipYUhTIQblgip5hNlNKH1EA16WBjBDM0Kos9EkcQ5%2BVZIBSsikPWxBcvusKLKEimPoAq9IfKGBhKvVcI5d%2FhmgZZnc2sxeOQDZYCtYhCMimILLHHfucEsLSVrQ9h1gYL7NLOm%2By5%2BruwSiMWZMJbHNzC3HCxsOrzTYeP988jb2u1PIE8KN84XUkH5clmQ34k69E8zHCEwoi3hxtQsTgdAV8Id9w13NZyCLMeyZvUvIYBzCQvd2bG7zjkwj5so3BHOIR8B%2BWLxPOCSaUX4pJLO6xZpQ87KwuFC1IzCYgQliAaQZpAGIgxRI5JjfrCGC0jHnH55aBXOFINVX23pAyuC2IpOkuIYkgowkfIq%2Fa0otJubvJMdHPPB8W8GD3pBBIHC53mnNX8Yb0lt5zf1E68u6%2F3RkX9E9r%2FNl6%2BDmHXdNTXPd7rLgOBhzNsksoC8mSxACPc2nBVcIj%2BZgvhKyk2f%2BGjCXSwOCRkbKvuU1pcifnv3TuRefMzboX2%2BLFiyTrbRG7y3TwdmEW7%2BWTRCebs9eRa%2FJIfVgDdEtyB6BLyGrMOkjHCbvVwkLiwHTT8RRiwNBTmTR0Ppeabgjib5CjyynDy6rCJn0%2FOasY5BVFju3VK0oNUKMoG0gWizUsjXmB6c4EjZBb54oicBcLBbllCjnAN4D6Ep%2BWIwgBYZyRSceyF%2FzjGztGKV0xRIY6mqIaEpLoF%2B5qGPKuClEJPkI8%2BQzlBSZisA36MvtlgHkqf%2B2GFPlrl73ewl9aFHgfy1U5Pd2X2EnPVcfzzkDlnbqgOMg7ryOaIaUgKQxYiSBf7%2Bchy3VLMJHd3M%2BpwlbpYKDQgYoMjHm9B8u%2BzKqSNbctmzLAoBr5u0jbH%2FmtlCpt80CAKC9mERFCDjERoq2wQK%2FMAo7KArauiGmBAzz8xZk5CfYdP7xa%2FUzcr2TeNW3F91OI1VwA42Aoavnc4Npas1hVZO0mVcW%2BCsY6VMI0ogX9mzst84L0aDcrSAoudTUuzWRvrVsqgFKg0rhuMSv1dTXv7Klb9hPciSubWq%2FWcdmhSma9SpexC7QVoG4j7K1K8jc1qxdTAxLzVGsXlfI1S5f%2FC8A2C8DqwuBdC0D9Atb4D5C%2F%2Fs3bXoxKH3eNM5N%2FSn5%2B54TgtpUQev%2B2hFDr6V8sIbiW%2FWEJ4Y9r8kCTye3o%2Bfs0dCc%2F6HaGNNuQc%2FG4UwYXgGcG45KiKOpYXLnx2%2Fkn%2Fp%2F%2F2WkfI06ea24ZyxBUQEVchZD%2FPxdXR%2Ffd6ZB%2FO2eGmIsEzcYi5oQwZEzsIg%2FFs1sTn8IX5HPyj0i8PiN0yaVSPUfQZJ5014B%2Fv%2BiafFIcyY3PKkuUUrrJUkYhN0lRlgYwXDBNEohQEOB9u65lBiwknsFJNj4dQ4WKq9v5bCNvaLGihtQtRcslpKkre1iYcM5x0FuKlqgsFJnYb6oK0%2BRTlX6DaWp%2BN2fPCWMkauTsNsqCCl33dUSgY4KTUYGr1oaa3cH32CvSFhP9htXEbrepOO0k1UTbO0x5McFZ1tEXEy1vdyvJ%2Fdjt7qZVwolqgNl976sB5t60f3M7%2BR76k%2FBmpslt%2BzjlAw9rDuLzmNK6Eaabnta0hGmn75Vrn92i%2Fa2YditUahyLacerV3QQ05miyntW1KiIf%2B1usVkJY3Ng1D5QdbzltLu9rA0%2FtVz4FcLvtenhZIelTVNKS%2BtT1x70yphpZ91pei1Fn6IoOxFpIaO8Nr567uviqzq%2B7fjyHxZP38itt5qh%2FtPkcng9vtKlNzWw6o5vYBwcf3izF%2BKvWAsblRLiHQ9FtAbV2O8di90dp9wXrhzaOTvBYWpTXqrDZMuHqWq82YMSdBxjcBzpVBW51Vy9h3QOxzDv5r8wS4fnv9Ozx%2F8A")
    
      INIT(
        Initialize,
        {
          reset();
        })

      TEST(
        FirstTime,
        Begin,
        CallsEntryAndDoit)
      {
        using namespace StatemachineEventTestImpl;

        Sm sm;
        auto result = sm.begin();
        EQ((uint8_t)2, result.activeState->getTypeId());
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
      }

      TEST(
        DuringExecution,
        Begin,
        ResetsStatemachine)
      {
        using namespace StatemachineEventTestImpl;

        Sm sm;
        sm.begin();

        auto result = sm.dispatch(Event::On{});
        EQ((uint8_t)1, result.activeState->getTypeId());

        reset();

        result = sm.begin();
        EQ((uint8_t)2, result.activeState->getTypeId());
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
      }

      TEST(
        SelfTransition,
        Dispatch,
        CallsDoit)
      {
        using namespace StatemachineEventTestImpl;

        Sm sm;
        sm.begin();

        reset();

        auto result = sm.dispatch(Event::Self{});
        EQ((uint8_t)2, result.activeState->getTypeId());
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
      }

      TEST(
        ReenterTransition,
        Dispatch,
        CallsExitEntryDoit)
      {
        using namespace StatemachineEventTestImpl;
        Sm sm;
        sm.begin();

        reset();

        auto result = sm.dispatch(Event::Reenter{});
        EQ((uint8_t)2, result.activeState->getTypeId());
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(1, OffState::exitCalls);
        EQ(1, OffState::entryCalls);
        EQ(1, OffState::doitCalls);
      }

      TEST(
        StatemachineNotInitialized,
        Dispatch,
        DoesNothing)
      {
        using namespace StatemachineEventTestImpl;

        Sm sm;
        auto result = sm.dispatch(Event::On{});
        N(result.activeState);
        FALSE(result.consumed);
      }

      TEST(
        ActiveStateHasFinalStateDefined,
        End,
        CallsExit)
      {
        using namespace StatemachineEventTestImpl;

        Sm sm;
        sm.begin();

        auto result = sm.dispatch(Event::On{});
        EQ((uint8_t)1, result.activeState->getTypeId());

        reset();

        result = sm.end();
        N(result.activeState);
        EQ(1, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
      }

      TEST(
        ActiveStateWithoutFinalState,
        End,
        DoesNotCallExit)
      {
        using namespace StatemachineEventTestImpl;

        Sm sm;
        sm.begin();

        reset();

        auto result = sm.end();
        NN(result.activeState); // State is still active (object is not terminated/destroyed)
        EQ(0, OnState::exitCalls);
        EQ(0, OnState::entryCalls);
        EQ(0, OnState::doitCalls);
        EQ(0, OffState::exitCalls);
        EQ(0, OffState::entryCalls);
        EQ(0, OffState::doitCalls);
      }

      TEST(
        StatemachineNotInitialized,
        End,
        DoesNothing)
      {
        using namespace StatemachineEventTestImpl;

        Sm sm;
        auto result = sm.end();
        N(result.activeState);
        FALSE(result.consumed);
      }

      void reset() const
      {
        using namespace StatemachineEventTestImpl;
        OnState::exitCalls = 0;
        OnState::entryCalls = 0;
        OnState::doitCalls = 0;
        OffState::exitCalls = 0;
        OffState::entryCalls = 0;
        OffState::doitCalls = 0;
      }

    END

  }
}
