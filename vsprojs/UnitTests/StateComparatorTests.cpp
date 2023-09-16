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
#include "../../src/state.h"
#include "../../src/lokilight.h"

namespace UT {
  namespace Classes {

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
        using Policy = T;
      };

      template<class T>
      struct TestStateSingletonB : T, SingletonCreator<TestStateSingletonB<T>> {
        using CreatorType = SingletonCreator<TestStateSingletonB<T>>;
        using Policy = T;
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

    BEGIN(StateComparatorTests)

      TEST(
        VirtualIdComparator,
        Equals,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<VirtualTypeIdComparator, false>>;
        using B = TestStateB<State<VirtualTypeIdComparator, false>>;

        A a1;
        A a2;
        B b1;
        B b2;

        FALSE(a1 == b1);
        TRUE(a1 == a2);
        TRUE(b1 == b1);

        FALSE(a1.equals(b1));
        TRUE(a1.equals(a2));
        TRUE(b1.equals(b2));
      }

      TEST(
        VirtualIdComparator,
        GetTypeId,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<VirtualTypeIdComparator, false>>;
        using B = TestStateB<State<VirtualTypeIdComparator, false>>;
        A a;
        B b;

        TRUE(a.typeOf<A>());
        TRUE(b.typeOf<B>());
      }

      TEST(
        MemoryAddressComparatorComparesSingletons,
        Equals,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        TestStateSingletonA<State<MemoryAddressComparator, true>> a;
        TestStateSingletonB<State<MemoryAddressComparator, true>> b;

        FALSE(a == b);
        TRUE(a == a);
        TRUE(b == b);

        FALSE(a.equals(b));
        TRUE(a.equals(a));
        TRUE(b.equals(b));
      }

      TEST(
        MemoryAddressComparator,
        GetTypeId,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateSingletonA<State<MemoryAddressComparator, true>>;
        using B = TestStateSingletonB<State<MemoryAddressComparator, true>>;
        A* a = A::create();
        B* b = B::create();

        TRUE(a->typeOf<A>());
        TRUE(b->typeOf<B>());
      }

      TEST(
        RttiComparator,
        Equals,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        TestStateA<State<RttiComparator, false>> a;
        TestStateB<State<RttiComparator, false>> b;

        FALSE(a == b);
        TRUE(a == a);
        TRUE(b == b);

        FALSE(a.equals(b));
        TRUE(a.equals(a));
        TRUE(b.equals(b));
      }

      TEST(
        RttiComparator,
        GetTypeId,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<RttiComparator, false>>;
        using B = TestStateB<State<RttiComparator, false>>;
        A a;
        B b;

        TRUE(a.typeOf<A>());
        TRUE(b.typeOf<B>());
      }

      TEST(
        TestComparator,
        Equals,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        TestStateA<State<TestComparator, false>> a;
        TestStateB<State<TestComparator, false>> b;

        TestComparator::isEqual = true;
        TRUE(a == b); // IsTrue because of the global isEqual flag
        TRUE(a == a);
        TRUE(b == b);

        TRUE(a.equals(b)); // IsTrue because of the global isEqual flag
        TRUE(a.equals(a));
        TRUE(b.equals(b));

        TestComparator::isEqual = false;
        FALSE(a == b);
        FALSE(a == a); // IsTrue because of the global isEqual flag
        FALSE(b == b);

        FALSE(a.equals(b)); // IsTrue because of the global isEqual flag
        FALSE(a.equals(a));
        FALSE(b.equals(b));
      }

      TEST(
        TestComparator,
        GetTypeId,
        ComparisonWorks)
      {
        using namespace StateComparatorTestsImpl;
        using A = TestStateA<State<TestComparator, false>>;
        using B = TestStateB<State<TestComparator, false>>;
        A a;
        B b;

        TestComparator::isEqual = true;
        TRUE(a.typeOf<A>());
        TRUE(b.typeOf<B>());

        TestComparator::isEqual = false;
        FALSE(a.typeOf<A>());
        FALSE(b.typeOf<B>());
      }

    END

  }
}
