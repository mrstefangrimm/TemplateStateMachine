#include "pch.h"
#include "CppUnitTest.h"

#include "tsmlib/state.h"
#include "tsmlib/templatemeta.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace tsmlib;

namespace unittests_tsm_create {

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
