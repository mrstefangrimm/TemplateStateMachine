/*
  Copyright 2022 Stefan Grimm

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
#include "..\..\src\templatemeta.h"

namespace UnitTests {

  using namespace Microsoft::VisualStudio::CppUnitTestFramework;
  using namespace tsmlib;
  using namespace Loki;

  namespace StateTests {

    template<typename T>
    struct TestStateA : T, FactorCreator<TestStateA<T>> {
      typedef typename FactorCreator<TestStateA> CreatorType;
      uint8_t getTypeId() const { return 1; }
      void _vexit() override { }
    };

    template<typename T>
    struct TestStateB : T, FactorCreator<TestStateB<T>> {
      typedef typename FactorCreator<TestStateB<T>> CreatorType;
      uint8_t getTypeId() const { return 2; }
      void _vexit() override { }
    };

    template<typename T>
    struct TestStateSingletonA : T, SingletonCreator<TestStateSingletonA<T>> {
      typedef typename SingletonCreator<TestStateSingletonA<T>> CreatorType;
      void _vexit() override { }
    };

    template<typename T>
    struct TestStateSingletonB : T, SingletonCreator<TestStateSingletonB<T>> {
      typedef typename SingletonCreator<TestStateSingletonB<T>> CreatorType;
      void _vexit() override { }
    };

    struct TestComerator {

      static bool isEqual;

      static bool areEqual(const State<TestComerator, false>& lhs, const State<TestComerator, false>& rhs) {
        return isEqual;
      }

      template<typename T>
      static bool hasType(const State<TestComerator, false>& me) {
        return isEqual;
      }
    };
    bool TestComerator::isEqual = false;

    TEST_CLASS(StateComperatorTests)
    {
    public:

      TEST_METHOD(Equals_VirtualGetTypeIdStateComperator_ComparisonWorks)
      {
        typedef TestStateA<State<VirtualGetTypeIdStateComperator, false>> A;
        typedef TestStateB<State<VirtualGetTypeIdStateComperator, false>> B;

        A a1;
        A a2;
        B b1;
        B b2;

        Assert::IsFalse(a1 == b1);
        Assert::IsTrue(a1 == a2);
        Assert::IsTrue(b1 == b1);

        Assert::IsFalse(a1.equals(b1));
        Assert::IsTrue(a1.equals(a2));
        Assert::IsTrue(b1.equals(b2));
      }

      TEST_METHOD(GetTypeId_VirtualGetTypeIdStateComperator_ComparisonWorks)
      {
        typedef TestStateA<State<VirtualGetTypeIdStateComperator, false>> A;
        typedef TestStateB<State<VirtualGetTypeIdStateComperator, false>> B;
        A a;
        B b;

        Assert::IsTrue(a.typeOf<A>());
        Assert::IsTrue(b.typeOf<B>());
      }

      TEST_METHOD(Equals_MemoryAddressComperatorComparesSingletons_ComparisonWorks)
      {
        TestStateSingletonA<State<MemoryAddressComperator, true>> a;
        TestStateSingletonB<State<MemoryAddressComperator, true>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
      }

      TEST_METHOD(GetTypeId_MemoryAddressComperator_ComparisonWorks)
      {
        typedef TestStateSingletonA<State<MemoryAddressComperator, true>> A;
        typedef TestStateSingletonB<State<MemoryAddressComperator, true>> B;
        A* a = A::create();
        B* b = B::create();

        Assert::IsTrue(a->typeOf<A>());
        Assert::IsTrue(b->typeOf<B>());
      }

      TEST_METHOD(Equals_RttiComperator_ComparisonWorks)
      {
        TestStateA<State<RttiComperator, false>> a;
        TestStateB<State<RttiComperator, false>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
      }

      TEST_METHOD(GetTypeId_RttiComperator_ComparisonWorks)
      {
        typedef TestStateA<State<RttiComperator, false>> A;
        typedef TestStateB<State<RttiComperator, false>> B;
        A a;
        B b;

        Assert::IsTrue(a.typeOf<A>());
        Assert::IsTrue(b.typeOf<B>());
      }

      TEST_METHOD(Equals_TestComperator_ComparisonWorks)
      {
        TestStateA<State<TestComerator, false>> a;
        TestStateB<State<TestComerator, false>> b;

        TestComerator::isEqual = true;
        Assert::IsTrue(a == b); // IsTrue because of the global isEqual flag
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsTrue(a.equals(b)); // IsTrue because of the global isEqual flag
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));

        TestComerator::isEqual = false;
        Assert::IsFalse(a == b);
        Assert::IsFalse(a == a); // IsTrue because of the global isEqual flag
        Assert::IsFalse(b == b);

        Assert::IsFalse(a.equals(b)); // IsTrue because of the global isEqual flag
        Assert::IsFalse(a.equals(a));
        Assert::IsFalse(b.equals(b));
      }

      TEST_METHOD(GetTypeId_TestComperator_ComparisonWorks)
      {
        typedef TestStateA<State<TestComerator, false>> A;
        typedef TestStateB<State<TestComerator, false>> B;
        A a;
        B b;

        TestComerator::isEqual = true;
        Assert::IsTrue(a.typeOf<A>());
        Assert::IsTrue(b.typeOf<B>());

        TestComerator::isEqual = false;
        Assert::IsFalse(a.typeOf<A>());
        Assert::IsFalse(b.typeOf<B>());
      }

    };
  }
}
