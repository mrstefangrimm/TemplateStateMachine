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
#include "../../src/state.h"
#include "../../src/lokilight.h"

namespace UT {
  namespace Classes {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;
    using namespace LokiLight;

    namespace StateComparatorTestsImpl {

      template<class T>
      struct TestStateA : T, FactoryCreator<TestStateA<T>> {
        using CreatorType = FactoryCreator<TestStateA<T>>;
        uint8_t getTypeId() const { return 1; }
      };

      template<class T>
      struct TestStateB : T, FactoryCreator<TestStateB<T>> {
        using CreatorType = FactoryCreator<TestStateB<T>>;
        uint8_t getTypeId() const { return 2; }
      };

      template<class T>
      struct TestStateSingletonA : T, SingletonCreator<TestStateSingletonA<T>> {
        using CreatorType = SingletonCreator<TestStateSingletonA<T>>;
      };

      template<class T>
      struct TestStateSingletonB : T, SingletonCreator<TestStateSingletonB<T>> {
        using CreatorType = SingletonCreator<TestStateSingletonB<T>>;
      };

      struct TestComparator {

        static bool isEqual;

        static bool areEqual(const State<TestComparator, false>&, const State<TestComparator, false>&) {
          return isEqual;
        }

        template<class T>
        static bool hasType(const State<TestComparator, false>&) {
          return isEqual;
        }
      };
      bool TestComparator::isEqual = false;
    }

    TEST_CLASS(StateComparatorTests)
    {
    public:
      TEST_METHOD(Equals_VirtualGetTypeIdStateComparator_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<VirtualGetTypeIdStateComparator, false>>;
        using B = TestStateB<State<VirtualGetTypeIdStateComparator, false>>;

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

      TEST_METHOD(GetTypeId_VirtualGetTypeIdStateComparator_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<VirtualGetTypeIdStateComparator, false>>;
        using B = TestStateB<State<VirtualGetTypeIdStateComparator, false>>;
        A a;
        B b;

        Assert::IsTrue(a.typeOf<A>());
        Assert::IsTrue(b.typeOf<B>());
      }

      TEST_METHOD(Equals_MemoryAddressComparatorComparesSingletons_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        TestStateSingletonA<State<MemoryAddressComparator, true>> a;
        TestStateSingletonB<State<MemoryAddressComparator, true>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
      }

      TEST_METHOD(GetTypeId_MemoryAddressComparator_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateSingletonA<State<MemoryAddressComparator, true>>;
        using B = TestStateSingletonB<State<MemoryAddressComparator, true>>;
        A* a = A::create();
        B* b = B::create();

        Assert::IsTrue(a->typeOf<A>());
        Assert::IsTrue(b->typeOf<B>());
      }

      TEST_METHOD(Equals_RttiComparator_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        TestStateA<State<RttiComparator, false>> a;
        TestStateB<State<RttiComparator, false>> b;

        Assert::IsFalse(a == b);
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsFalse(a.equals(b));
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));
      }

      TEST_METHOD(GetTypeId_RttiComparator_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<RttiComparator, false>>;
        using B = TestStateB<State<RttiComparator, false>>;
        A a;
        B b;

        Assert::IsTrue(a.typeOf<A>());
        Assert::IsTrue(b.typeOf<B>());
      }

      TEST_METHOD(Equals_TestComparator_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        TestStateA<State<TestComparator, false>> a;
        TestStateB<State<TestComparator, false>> b;

        TestComparator::isEqual = true;
        Assert::IsTrue(a == b); // IsTrue because of the global isEqual flag
        Assert::IsTrue(a == a);
        Assert::IsTrue(b == b);

        Assert::IsTrue(a.equals(b)); // IsTrue because of the global isEqual flag
        Assert::IsTrue(a.equals(a));
        Assert::IsTrue(b.equals(b));

        TestComparator::isEqual = false;
        Assert::IsFalse(a == b);
        Assert::IsFalse(a == a); // IsTrue because of the global isEqual flag
        Assert::IsFalse(b == b);

        Assert::IsFalse(a.equals(b)); // IsTrue because of the global isEqual flag
        Assert::IsFalse(a.equals(a));
        Assert::IsFalse(b.equals(b));
      }

      TEST_METHOD(GetTypeId_TestComparator_ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<TestComparator, false>>;
        using B = TestStateB<State<TestComparator, false>>;
        A a;
        B b;

        TestComparator::isEqual = true;
        Assert::IsTrue(a.typeOf<A>());
        Assert::IsTrue(b.typeOf<B>());

        TestComparator::isEqual = false;
        Assert::IsFalse(a.typeOf<A>());
        Assert::IsFalse(b.typeOf<B>());
      }
    };
  }
}
