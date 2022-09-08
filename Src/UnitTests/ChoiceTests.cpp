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

#include "tsmlib\state.h"
#include "tsmlib\choice.h"

namespace UnitTests {

  using namespace tsmlib;
  using namespace Microsoft::VisualStudio::CppUnitTestFramework;

  struct TestObject { };
  typedef State<MemoryAddressStateComperator<true>, true> StateType;

  class Init : public SimpleState<Init, StateType>, public SingletonCreator<Init> {
    friend class SimpleState<Init, StateType>;
    void entry_() { }
    void exit_() { }
    template<uint8_t N>
    void doit_() { }
  };

  class ChoiceTrue : public SimpleState<ChoiceTrue, StateType>, public SingletonCreator<ChoiceTrue> {
    friend class SimpleState<ChoiceTrue, StateType>;
    void entry_() { }
    void exit_() { }
    template<uint8_t N>
    void doit_() { }
  };

  class ChoiceFalse : public SimpleState<ChoiceFalse, StateType>, public SingletonCreator<ChoiceFalse> {
    friend class SimpleState<ChoiceFalse, StateType>;
    void entry_() { }
    void exit_() { }
    template<uint8_t N>
    void doit_() { }
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
      // Note: SingletonCreator requires to use the factories.
      typedef typename ChoiceTrue::CreatorType TrueFactory;
      ChoiceTrue* trueState = TrueFactory::create();
      typedef typename Init::CreatorType InitFactory;
      Init* init = InitFactory::create();

      GuardDummy::CheckReturnValue = true;

      InitChoice choice;
      StateType* state = choice.dispatch(init);

      Assert::IsTrue(*trueState == *state);
    }

    TEST_METHOD(Check_ReturnsFalse_FalseState)
    {
      // Note: SingletonCreator requires to use the factories.
      typedef typename ChoiceFalse::CreatorType FalseFactory;
      ChoiceFalse* falseState = FalseFactory::create();
      typedef typename Init::CreatorType InitFactory;
      Init* init = InitFactory::create();

      GuardDummy::CheckReturnValue = false;

      InitChoice choice;
      StateType* state = choice.dispatch(init);

      Assert::IsTrue(*falseState == *state);
    }
  };
}
