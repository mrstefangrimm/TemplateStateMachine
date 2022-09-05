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

	template<typename T>
	struct TestStateA : T {
		uint8_t getTypeId() const override { return 1; }
	};

	template<typename T>
	struct TestStateB : T {
		uint8_t getTypeId() const override { return 2; }
	};

	template<typename T>
	struct TestStateMinimalA : T {
	};

	template<typename T>
	struct TestStateMinimalB : T {
	};

	TEST_CLASS(StateComperatorTests)
	{
	public:
		
		TEST_METHOD(Equals_ComperatorStateless_ComparisonWorks)
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

		TEST_METHOD(Equals_ComperatorSingleton_ComparisonWorks)
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

		TEST_METHOD(Equals_ComperatorSingletonAndMinimal_ComparisonWorks)
		{
			TestStateMinimalA<State<MemoryAddressStateComperator<true>, true>> a;
			TestStateMinimalB<State<MemoryAddressStateComperator<true>, true>> b;

			Assert::IsFalse(a == b);
			Assert::IsTrue(a == a);
			Assert::IsTrue(b == b);

			Assert::IsFalse(a.equals(b));
			Assert::IsTrue(a.equals(a));
			Assert::IsTrue(b.equals(b));
		}

		/*TEST_METHOD(Equals_ComperatorRtti_ComparisonWorks)
		{
			TestStateA<State<TypeidStateComperator, false>> a;
			TestStateB<State<TypeidStateComperator, false>> b;

			Assert::IsFalse(a == b);
			Assert::IsTrue(a == a);
			Assert::IsTrue(b == b);

			Assert::IsFalse(a.Equals(b));
			Assert::IsTrue(a.Equals(a));
			Assert::IsTrue(b.Equals(b));
		}*/

	};
}
