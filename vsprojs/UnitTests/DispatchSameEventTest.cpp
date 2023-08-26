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

#include "../../src/tsm.h"
#include "TestHelpers.h"

namespace UT {
  namespace Transitions {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace UnitTests::Helpers;

    namespace DispatchSameEventTestImpl {

      typedef State<VirtualGetTypeIdStateComparator, false> StateType;
      typedef FactoryCreatorFake<StateType> StateTypeCreationPolicyType;
      typedef Recorder<sizeof(__FILE__) + __LINE__> RecorderType;
      template<class Derived> struct Leaf : BasicState<Derived, StateType>, FactoryCreatorFake<Derived> {};

      template<class From>
      struct ActionChoiceRecordingSpy {
        template<class StateType, class EventType>
        void perform(StateType*, const EventType&) {
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

        void entry() { RecorderType::add("A::Entry"); }
        void exit() { RecorderType::add("A::Exit"); }
        template<class Event>
        void doit(const Event& ev) {
          RecorderType::add("A::Do");
          counter++;
        }
        uint8_t getTypeId() const override { return 1; };
      };
      const char* A::name = "A";

      struct B : Leaf<B> {
        static const char* name;
        int counter = 0;
        
        void entry() { RecorderType::add("B::Entry"); }
        void exit() { RecorderType::add("B::Exit"); }
        template<class Event>
        void doit(const Event& ev) {
          RecorderType::add("B::Do");
          counter++;
        }

        uint8_t getTypeId() const override { return 2; };
      };
      const char* B::name = "B";

      struct GotoBCountGuardA {
        template<class StateType, class EventType>
        bool eval(StateType* state, const EventType& ev) {
          return static_cast<A*>(state)->counter > 2;
        }
      };

      struct GotoACountGuardB {
        template<class StateType, class EventType>
        bool eval(StateType* state, const EventType& ev) {
          return static_cast<B*>(state)->counter > 2;
        }
      };

      typedef ChoiceTransition<Trigger::Count, B, A, A, StateTypeCreationPolicyType, GotoBCountGuardA, ActionChoiceRecordingSpy<A>> ToBorSelfA;
      typedef ChoiceTransition<Trigger::Count, A, B, B, StateTypeCreationPolicyType, GotoACountGuardB, ActionChoiceRecordingSpy<B>> ToAorSelfB;
      typedef
        Typelist<ToBorSelfA,
        Typelist<ToAorSelfB,
        NullType>> Transitions;

      typedef InitialTransition<A, StateTypeCreationPolicyType, ActionSpy<A, InitialStateNamedFake<StateType>, RecorderType>> ToplevelInitTransition;
      typedef Statemachine<
        Transitions,
        ToplevelInitTransition> Sm;
    }

    // https://viewer.diagrams.net/?tags=%7B%7D&highlight=0000ff&edit=_blank&layers=1&nav=1&title=DispatchSameEventTest.drawio#R7Vpdc9o4FP01zGwfyPgbeAxfaZu2k1kybdk3YQtbrWxRWQTMr68EMliWAYeaJLuzPCTStSTb95xzda%2BgZQ%2Fi9R0Fi%2BgzCSBuWUawbtnDlmWZhmHwf8KS7Sxer7szhBQFctDBMEEbmM%2BU1iUKYKoMZIRghhaq0SdJAn2m2AClZKUOmxOs3nUBQqgZJj7AuvUbCli0s3Zd42B%2FD1EY5Xc28xeOQT5YGtIIBGRVMNmjlj2ghLBdK14PIBbOy%2F2ymzc%2BcnX%2FYBQmrM4Etnp4hDhjo9sPq3Uy2jw8jbptucoTwEv5wmksH5dluQ%2F4ky9EcxnjMQUxb%2FZXEWJwsgC%2BsK849NwWsRjznsmblCyTAAayt39zg3d8EiNftjGYQdwH%2Fs9wO2FAMKH8UkIScY%2BUUfJz73Wx0JwkbAxihAWZvkIagARIs2SOyd3aBxiFCe%2F43DOQL9jXXZW%2FN6QMrgsm6bo7SGLIaMaHyKu2RDFTu6sDJ3o5J6ICH1xHGoHkYbhf%2BQAVb0i0noGcpSF3qwGnwnAGM0B96UTLEU5FGOeAtCx7zj%2B%2BsUdFuWIIlV8Mc01QRb9wV8OQd9XAFpgiLuBbeYGRRTMUsByVA12dA%2FshRQ7YxrU4YGsc6P%2FPgatywHtzHOhqiMOA72CySyiLSEgSgEcHayE8C%2B8dxnwiwk9bJvyAjGXS92DJiMqTo55MyZL68MTj9uQeDmgI2fnwJt7lJC4UYsDQk7pbN%2B7kniY03esY85wEqn7Kd86Uvy%2BbMMCgrqo9i4%2BrqpHttGmNBYjyhAsRYeSwC%2Fo3oTDTUxXm6AqzKwV2JehNR8N%2BwL3LTgTaC0XFfUaz73L%2BtjMVnRs37w7XxYvDLO%2BtEfuer8Hbu1kdV3YPs0Qnn3S5gm1dwV38yfnqZNh3%2FOjD4lfmfiGz2gqWuLeNG5N%2FFOzzIqC2yOXqDwQJgPZDyHyeQqZRYf8Qtdhx6iVPBQYakXi2TM9vvg2Ip%2BO4igOt3supp9I%2Fru4fl8es5TaGiILFA7FwQ8i2DjD45eEVlCUVYigKOS2QvRpNVY2dc3KspaxTZGpsbzwoy%2FMMRyFG23T%2FTFpybaenRmuzV2LS7qXlrGJ1WlrIdc8stPPKiYWuL%2FaKrUDj6rkEO13sTinmaC0Y3Yjo3ZLoc2wLojcrRG82IHoEO9Sa%2FppOJt%2F67ub9vR%2FfDysOFWbicScMzkHCG3cUxXHL4osbfw3eCdUblr3rY8Tzi5R7xjKE5kWMYBHkfwfian%2FanvAq13BuDDEXiSiSCKkIY8SYOBO6Fc9ujX0Kt7zm%2BVFMkvSG0JBb5fIpb86ydgr4%2F%2B1a43cakNz5rJTMKflZnlUVkjlpyjMlDOesohaJURDgY2coaqgr5Ga9qxxjuBX1i2tUcMW6Flf0DfSRojCEdAelh4ULZ5wH3naLqE69robVjDBG4lpYNQBOqbZ0XB0cswocswklf%2FH7OIt608d%2FnI%2BE%2BPNN%2BIQrlPyq6Y1dSm%2FMFywOKv2js%2FeN1gZq5mNdqzaoyGBO8eoZGYzhdDwV%2Bz%2BsDfJDvXLe4bnPTGCO5x3azezSTq3d7EiSc0ECU%2Bn2ivzl%2Bdn4CxwvHWO7dVkp3ADbT7H4bL5uP5vtbtfpqUxpJl23y4cr5Z2jbrreLa3TfdVsvRKduqXnNsPYDjjC9yS4FV8kHpKGyi%2B6lJqxEJinRU6fi9J7qk6LTG2Ytt7VgrR2gNOuKEUqvyZrJpiXq1FeGl1Gb7u0kG3UC9ScKCArDFuIAelrKcDTFfAGytXyJsjR1zhyrXJ1Pv7oTDbe8Mfn9TreROEa%2F31fe1f8LweKuudRJ89F%2FkWRolOOFOUNrG6k0Bay3lak4N3DT0x2ww8%2F1LFHvwE%3D
    TEST_CLASS(DispatchSameEventTest)
    {
    public:
      TEST_METHOD_INITIALIZE(Initialize)
      {
        using namespace DispatchSameEventTestImpl;
        RecorderType::reset();
        FactoryCreatorFake<A>::reset();
        FactoryCreatorFake<B>::reset();
      }

      TEST_METHOD(Count_WhenCounterOverThreshold_ThenB)
      {
        using namespace DispatchSameEventTestImpl;
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
          "B::Do" });


        // Active state is B
        Assert::AreNotEqual<int>(0, FactoryCreatorFake<B>::createCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls + 1);
      }

      TEST_METHOD(Count_WhenCounterOverThreshold_ThenBthenA)
      {
        using namespace DispatchSameEventTestImpl;
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
          "B::Do" });

        sm.dispatch<Trigger::Count>();
        RecorderType::check({
          "?<-B",
          "B::Do" });

        sm.dispatch<Trigger::Count>();
        RecorderType::reset();
        sm.dispatch<Trigger::Count>();
        RecorderType::check({
          "?<-B",
          "B::Exit",
          "A::Entry",
          "A::Do" });

        // Active state is B
        Assert::AreNotEqual<int>(0, FactoryCreatorFake<B>::createCalls);
        Assert::AreEqual<int>(FactoryCreatorFake<A>::createCalls, FactoryCreatorFake<A>::deleteCalls + 1);
        Assert::AreEqual<int>(FactoryCreatorFake<B>::createCalls, FactoryCreatorFake<B>::deleteCalls);
      }

    };
  }
}
