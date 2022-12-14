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

  namespace StateCreation {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;

    struct TestObject { };

    TEST_CLASS(CreatorTests)
    {
    public:

      TEST_METHOD(Create_SingletonCreator_PointsToSameObject)
      {
        TestObject* obj1 = SingletonCreator<TestObject>::create();
        TestObject* obj2 = SingletonCreator<TestObject>::create();

        Assert::IsTrue(obj1 == obj2);

        SingletonCreator<TestObject>::destroy(obj1);
        SingletonCreator<TestObject>::destroy(obj2);
      }

      TEST_METHOD(Create_FactoryCreator_PointsToDifferentObjects)
      {
        TestObject* obj1 = FactorCreator<TestObject>::create();
        TestObject* obj2 = FactorCreator<TestObject>::create();

        Assert::IsFalse(obj1 == obj2);

        FactorCreator<TestObject>::destroy(obj1);
        FactorCreator<TestObject>::destroy(obj2);
      }
    };
  }
}
