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

namespace UT {
  namespace Classes {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;

    namespace EventDispatchersTestImpl {

      using StatePolicy = State<VirtualGetTypeIdStateComparator, false>;

      namespace Trigger
      {
        struct On {};
        struct Off {};
        struct Timeout {};
        struct Wrong {};
      }

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

      struct WrongState : StatePolicy, FactoryCreator<OnState> {
        using Policy = StatePolicy;
        uint8_t getTypeId() const override { return 3; }
      };

      using ToOnFromOffTransition = Transition<Trigger::On, OnState, OffState, NoGuard, NoAction>;
      using ToOffFromOnTransition = Transition<Trigger::Off, OffState, OnState, NoGuard, NoAction>;
      using ToOnFromOnTransition = SelfTransition<Trigger::Timeout, OnState, NoGuard, NoAction, false>;
      using ToOffFromOffTransition = SelfTransition<Trigger::Timeout, OffState, NoGuard, NoAction, false>;

      using TransitionList =
        Typelist<ToOnFromOffTransition,
        Typelist<ToOffFromOnTransition,
        Typelist<ToOnFromOnTransition,
        Typelist<ToOffFromOffTransition,
        NullType>>>>;
    }

    // https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&layers=1&nav=1&title=TriggerExecutorTest.drawio#R7Vlbc%2BI2FP41zHQfyPiCCXkEAtmd2W3TkubSl46wha1GtlhZBJxfv0dYNrYlCJuYbDtdMgPS0cXWOZ%2B%2B80npuON4c8XRMvrCAkw7jhVsOu5lx3EGPQe%2BpSHLDeeOnRtCToLcVDHMyDNWRktZVyTAaa2jYIwKsqwbfZYk2Bc1G%2BKcrevdFozWn7pEIdYMMx9R3XpHAhGpZXnWzv4RkzAqnmxbqiVGRWdlSCMUsHXF5E467pgzJvJSvBljKn1X%2BCUfN93TWr4Yx4k4ZoBYX99gmonJ8NN6k0yer58mg66a5QnRlVpwGqvXFVnhA3jzpSyuYjrlKIbiaB0RgWdL5Ev7GiIPtkjEFGo2FDlbJQEOVK1cuQUVn8XEV2WK5piOkP8YbgeMGWUcmhKWyGekgrPH0utyogVLxBTFhEow3WIeoAQps0KODW4dIUrCBCo%2BeAbDhCPdVcW6MRd4UzEp111hFmPBM%2BiiWl0VxaxeXVcwUUA2quDBLToihcOwnHkXKiioaJkj9zfno8HDH%2FfWJf%2FLmXx0Hr%2FOcdceaKH7bbHQYocDgLOqMi4iFrIE0cnOWomVDMmuz2fGlsrv%2F2AhMuVhtBKsHmxwKs%2Fu1fht5UFWzryiermpNl5mRW1DxH0xB5Qro6C2GyQrxZi9kUzZivv4ANIVDwnEQywO%2BPUi7yf9dhAXCgi2l%2FfnmCJBnuqsYQq6mumaEVjBDl69Or6cfgM3%2BfrUqOoub0zUcweHJ8odcGCioiNbLFJc67PFaemCo6B7KBRV5CYacOsU8gLfIO4reDo9SQiE0oJMOo67gI9vlYxSa7Hg83qKOpKQZL3yVMtST9WISvIRgeQzVA1CbsE26Ms%2Br%2BNroPNX2aXKX2X2egt%2FGUEw%2BLFUtWOnhxo5manq9bRzodPOoT3xIu18H88MOUdZpcNS7vF0Pw05nleDiaru4pxP2CobXGhsoCODUtB7uB7LQpWk4FsxE0hgfeeXO23%2Fzm9FqrTNAwHhIGYJk0aAmNyirbBAv84CPZ0FXJOIaYEDBvRz77aXUb%2FnR5%2BWXzPvVzbv2q4W%2BxsSY1iwDoEkGEo5v%2FO5UW5WdUVRPkZX7NMwzksi5ihmMC%2B%2B1zI1qKB2C0lSiapniGphe6tyaWBKQ8vRysVuSOxm6tmjXPZz3Im1zcGoHqKzl8RMusxPsguykaBuY%2Bc7jfxvGw4wpvOLfarji876ptPLTxHYpghsng3eVQSaD7HW%2FyMBmBff9pFUhblrndnwqYX6nZOC11ZS6P%2FXksLBSP%2FLkoLnuD8sKfz%2BhT3ybHozev5zFnnTO765JYbbyLl83ZnAC5RA4YqTOO44MLn1y%2FgDfMOfm9cpAf5MwTOOJamAy30VYfgey9bRQ3c2hN%2FemSXHEsm0idxz0hgJIS%2BTh%2FLdnanP8Rb5wP8xS9IzxkOwqukBQdN51k0R%2FG7nmn7QAgnOF42TSi3jFFmjkp6UqcgEFC%2BEIQ%2FEJAjovsvXOgNWcs9FiRUNGAb4HH3%2Fea5DxbNMueNUWNG31A0nYYh5Hso%2BlS6cAw76oSzJGybNJhVH01jmn2bDHWdJ%2BH7RnjMhWHxUtFsIb%2FP60XAytE3htU%2FGBZ4uEPVLwve4MjKKifMj1UR56VQd9qKaaGT618mL77p5OiQmgGV7ZjHR8qW3ltxfe%2Bl9rEo4kQYw4vlcw7OZUH4eeTr1I8%2BbeK3fyFunPPJAdfff3Bw2u3%2BJu5Nv
    TEST_CLASS(EventDispatchersTest)
    {
      TEST_METHOD_INITIALIZE(Initialize)
      {
        using namespace EventDispatchersTestImpl;
        OnState::exitCalls = 0;
        OnState::entryCalls = 0;
        OnState::doitCalls = 0;
        OffState::exitCalls = 0;
        OffState::entryCalls = 0;
        OffState::doitCalls = 0;
      }

      TEST_METHOD(Execute_ActiveStateOnTriggerTimeout_CallsOnStateDoit)
      {
        using namespace EventDispatchersTestImpl;
 
        OnState on;
        Trigger::Timeout t;

        const int size = Length<TransitionList>::value;
        auto result = EventDispatcher<TransitionList, Trigger::Timeout, size - 1>::execute(&on, &t);
        Assert::IsTrue(result.consumed);
        Assert::AreEqual<int>(on.getTypeId(), result.activeState->getTypeId());

        Assert::AreEqual<int>(1, OnState::doitCalls);
        Assert::AreEqual<int>(0, OffState::doitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOnTriggerOn_UnhandledTransition)
      {
        using namespace EventDispatchersTestImpl;

        OnState on;
        Trigger::On t;

        const int size = Length<TransitionList>::value;
        auto result = EventDispatcher<TransitionList, Trigger::On, size - 1>::execute(&on, &t);
        Assert::IsFalse(result.consumed);
        Assert::IsNull(result.activeState);

        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
      }

      TEST_METHOD(Execute_WrongActiveStateTriggerTimeout_UnhandledTransition)
      {
        using namespace EventDispatchersTestImpl;

        WrongState wrongState;
        Trigger::Timeout t;

        const int size = Length<TransitionList>::value;
        auto result = EventDispatcher<TransitionList, Trigger::Timeout, size - 1>().execute(&wrongState, &t);
        Assert::IsFalse(result.consumed);
        Assert::IsNull(result.activeState);

        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOnTriggerWrong_UnhandledTransition)
      {
        using namespace EventDispatchersTestImpl;

        OnState on;
        Trigger::Wrong t;

        const int size = Length<TransitionList>::value;
        auto result = EventDispatcher<TransitionList, Trigger::Wrong, size - 1>().execute(&on, &t);
        Assert::IsFalse(result.consumed);
        Assert::IsNull(result.activeState);

        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOffTriggerTimeout_CallsOffStateDoit)
      {
        using namespace EventDispatchersTestImpl;

        OffState off;
        Trigger::Timeout t;

        const int size = Length<TransitionList>::value;
        auto result = EventDispatcher<TransitionList, Trigger::Timeout, size - 1>::execute(&off, &t);
        Assert::IsTrue(result.consumed);
        Assert::AreEqual<int>(off.getTypeId(), result.activeState->getTypeId());

        Assert::AreEqual<int>(1, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
      }

      TEST_METHOD(Execute_ActiveStateOffTriggerOff_UnhandledTransition)
      {
        using namespace EventDispatchersTestImpl;

        OffState off;
        Trigger::Off t;

        const int size = Length<TransitionList>::value;
        auto result = EventDispatcher<TransitionList, Trigger::Off, size - 1>::execute(&off, &t);
        Assert::IsFalse(result.consumed);
        Assert::IsNull(result.activeState);

        Assert::AreEqual<int>(0, OffState::doitCalls);
        Assert::AreEqual<int>(0, OnState::doitCalls);
      }
    };
  }
}
