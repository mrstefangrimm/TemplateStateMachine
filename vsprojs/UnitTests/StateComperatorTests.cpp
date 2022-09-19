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
      uint8_t getTypeId() const override { return 1; }
      void _vexit() override { }
    };

    template<typename T>
    struct TestStateB : T, FactorCreator<TestStateB<T>> {
      typedef typename FactorCreator<TestStateB<T>> CreatorType;
      uint8_t getTypeId() const override { return 2; }
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

    TEST_CLASS(StateComperatorTests)
    {
    public:

      TEST_METHOD(Equals_VirtualGetTypeIdStateComperator_ComparisonWorks)
      {
        TestStateA<State<VirtualGetTypeIdStateComperator, false>> a;
        TestStateB<State<VirtualGetTypeIdStateComperator, false>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
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

      TEST_METHOD(Equals_MemoryAddressStateComperator_ComparisonWorks)
      {
        TestStateA<State<MemoryAddressStateComperator<false>, false>> a;
        TestStateB<State<MemoryAddressStateComperator<false>, false>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
      }

      TEST_METHOD(Equals_MemoryAddressStateComperatorAndSingleton_ComparisonWorks)
      {
        TestStateSingletonA<State<MemoryAddressStateComperator<true>, true>> a;
        TestStateSingletonB<State<MemoryAddressStateComperator<true>, true>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
      }

      TEST_METHOD(GetTypeId_ComperatorSingleton_ComparisonWorks)
      {
        typedef TestStateSingletonA<State<MemoryAddressStateComperator<true>, true>> A;
        typedef TestStateSingletonB<State<MemoryAddressStateComperator<true>, true>> B;
        A* a = A::create();
        B* b = B::create();

        Assert::IsTrue(a->typeOf<A>());
        Assert::IsTrue(b->typeOf<B>());
      }

      TEST_METHOD(Equals_TypeidStateComperator_ComparisonWorks)
      {
        TestStateA<State<TypeidStateComperator, false>> a;
        TestStateB<State<TypeidStateComperator, false>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
      }

      TEST_METHOD(GetTypeId_TypeidStateComperator_ComparisonWorks)
      {
        typedef TestStateA<State<TypeidStateComperator, false>> A;
        typedef TestStateB<State<TypeidStateComperator, false>> B;
        A a;
        B b;

        Assert::IsTrue(a.typeOf<A>());
        Assert::IsTrue(b.typeOf<B>());
      }

    };
  }
}
