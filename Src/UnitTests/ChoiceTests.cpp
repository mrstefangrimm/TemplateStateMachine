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

using namespace std;
#include "tsmlib/choice.h"
#include "tsmlib/templatemeta.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace tsmlib;

namespace UnitTests {

	struct TestObject { };
	typedef State<VirtualGetTypeIdStateComperator, false> StateType;

	struct Init : StateType, SingletonCreator<Init> {
		uint8_t getTypeId() const override { return 10; }
		void entry() { }
		void exit() { }
		template<uint8_t TRIGGER>
		void doit() {
		}
	};

	struct ChoiceTrue : StateType, SingletonCreator<ChoiceTrue> {
		uint8_t getTypeId() const override { return 11; }
		void entry() { }
		void exit() { }
		template<uint8_t TRIGGER>
		void doit() {
		}
	};

	struct ChoiceFalse : StateType, SingletonCreator<ChoiceFalse> {
		uint8_t getTypeId() const override { return 12; }
		void entry() { }
		void exit() { }
		template<uint8_t TRIGGER>
		void doit() {
		}
	};

	struct GuardDummy {
		static bool CheckReturnValue;
		template<typename T>
		bool check(T*) {
			return CheckReturnValue;
		}
	};
  bool GuardDummy::CheckReturnValue = false;

	typedef Choice<0, StateType, ChoiceTrue, ChoiceFalse, Init, GuardDummy, EmptyAction> InitChoice;

	TEST_CLASS(ChoiceTests)
	{
	public:
		
		TEST_METHOD(Check_ReturnsTrue_TrueState)
		{
			ChoiceTrue trueState;
			Init init;
			GuardDummy::CheckReturnValue = true;
			
			InitChoice choice;
			StateType* state = choice.trigger(&init);

			Assert::IsTrue(trueState == *state);
		}

		TEST_METHOD(Check_ReturnsFalse_FalseState)
		{
			ChoiceFalse falseState;
			Init init;
			GuardDummy::CheckReturnValue = false;

			InitChoice choice;
			StateType* state = choice.trigger(&init);

			Assert::IsTrue(falseState == *state);
		}
	};
}
