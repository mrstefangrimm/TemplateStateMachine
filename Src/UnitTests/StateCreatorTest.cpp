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
#include "CppUnitTest.h"

#include "tsmlib/state.h"
#include "tsmlib/templatemeta.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace tsmlib;

namespace UnitTests {

	struct TestObject { };
	TestObject singleObject;

	TEST_CLASS(StateCreatorTest)
	{
	public:
		
		TEST_METHOD(Create_SingletonCreator_PointsToSameObject)
		{
			TestObject* obj1 = SingletonCreator<TestObject>::Create();
			TestObject* obj2 = SingletonCreator<TestObject>::Create();

			Assert::IsTrue(obj1 == obj2);

			SingletonCreator<TestObject>::Delete(obj1);
			SingletonCreator<TestObject>::Delete(obj2);
		}

		TEST_METHOD(Create_FactoryCreator_PointsToDifferentObjects)
		{
			TestObject* obj1 = FactorCreator<TestObject>::Create();
			TestObject* obj2 = FactorCreator<TestObject>::Create();

			Assert::IsFalse(obj1 == obj2);

			FactorCreator<TestObject>::Delete(obj1);
			FactorCreator<TestObject>::Delete(obj2);
		}

	};
}
