#pragma once

#define IAMWORKSTATION 1
#include "..\..\src\tsm.h"

namespace UnitTests {
  namespace Helpers {

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;

    template<typename To, typename From>
    struct ActionSpy {
      static int Calls;
      template<typename T>
      void perform(T*) { Calls++; }
    };
    template<typename To, typename From> int ActionSpy<To, From>::Calls = 0;

    template<typename StateType, typename TO, typename FROM>
    struct GuardDummy {
      static int Calls;
      static bool CheckReturnValue;
      template<typename T>
      bool check(T* activeState) {
        if (!is_same < FROM, AnyState<StateType>>().value) {
          FROM* from = FROM::CreatorType::create();
          Assert::IsTrue(activeState->equals(*from));
          FROM::CreatorType::destroy(from);
        }
        Calls++;
        return CheckReturnValue;
      }
    };
    template<typename StateType, typename To, typename From> int GuardDummy<StateType, To, From>::Calls = 0;
    template<typename StateType, typename To, typename From> bool GuardDummy<StateType, To, From>::CheckReturnValue = true;

  }
}
