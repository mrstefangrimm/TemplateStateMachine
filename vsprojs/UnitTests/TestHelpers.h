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

    template<typename StateType, typename To, typename From>
    struct GuardDummy {
      static int Calls;
      static bool CheckReturnValue;
      template<typename T>
      bool eval(T* activeState) {
        if (!is_same < From, AnyState<StateType>>().value) {
          From* from = From::CreatorType::create();
          Assert::IsTrue(activeState->equals(*from));
          From::CreatorType::destroy(from);
        }
        Calls++;
        return CheckReturnValue;
      }
    };
    template<typename StateType, typename To, typename From> int GuardDummy<StateType, To, From>::Calls = 0;
    template<typename StateType, typename To, typename From> bool GuardDummy<StateType, To, From>::CheckReturnValue = true;

    template<typename StateType>
    struct InitialStateNamedFake : StateType {
      static const char* Name;
    };
    template<typename StateType> const char* InitialStateNamedFake<StateType>::Name = "Initial";

    template<typename T>
    struct FactorCreatorFake {
      typedef FactorCreatorFake<T> CreatorType;
      typedef T ObjectType;

      static int CreateCalls;
      static int DeleteCalls;

      static void reset() { CreateCalls = 0; DeleteCalls = 0; }

      static T* create() { CreateCalls++;  return new T; }
      static void destroy(T* state) { DeleteCalls++;  delete state; }
    };
    template<typename T> int FactorCreatorFake<T>::CreateCalls = 0;
    template<typename T> int FactorCreatorFake<T>::DeleteCalls = 0;

  }
}
