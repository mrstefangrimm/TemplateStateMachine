#pragma once

#define IAMWORKSTATION 1
#include "..\..\src\tsm.h"
#include <vector>

namespace UnitTests {
  namespace Helpers {

    // Dummy, Fake and Stub as suggested by Martin Fowler's article on Test Doubles: https://martinfowler.com/bliki/TestDouble.html?ref=net-from-the-mountain

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;

    template<typename To, typename From>
    struct ActionStub {
      static int calls;
      template<typename State>
      void perform(State*) { calls++; }
    };
    template<typename To, typename From> int ActionStub<To, From>::calls = 0;

    template<size_t TestId>
    class Recorder {
    private:
      static int checkedPosition_;
      static vector<string> records_;

    public:
      static void add(string record) { records_.push_back(record); }
      static void reset() { checkedPosition_ = 0; records_.clear(); }

      static void check(vector<string> expected) {
        for (int n = 0; n < expected.size(); n++) {
          string exp = expected[n];
          string act = Recorder::records_[checkedPosition_ + n];
          Assert::AreEqual<string>(exp, act);
        }
        checkedPosition_ += expected.size();
      }
      static void check(const char* expected[]) {
        vector<string> exp(begin(expected), end(expected));
        check(exp);
      }

      static void checkUnchanged() {
        Assert::AreEqual<size_t>(checkedPosition_, records_.size());
      }
    };
    template<size_t TestId> int Recorder<TestId>::checkedPosition_;
    template<size_t TestId> vector<string> Recorder<TestId>::records_;

    template<typename To, typename From, typename Recorder>
    class ActionSpy {
    public:
      static int calls;

      template<typename State>
      void perform(State*) {
        calls++;
        ostringstream buf;
        buf << To::name << "<-" << From::name;
        Recorder::add(buf.str());
      }
    };
    template<typename To, typename From, typename Recorder> int ActionSpy<To, From, Recorder>::calls = 0;

    template<typename StateType, typename To, typename From>
    struct GuardDummy {
      static int calls;
      static bool CheckReturnValue;
      template<typename T>
      bool eval(T* activeState) {
        if (!is_same < From, AnyState<StateType>>().value) {
          From* from = From::CreatorType::create();
          Assert::IsTrue(activeState->equals(*from));
          From::CreatorType::destroy(from);
        }
        calls++;
        return CheckReturnValue;
      }
    };
    template<typename StateType, typename To, typename From> int GuardDummy<StateType, To, From>::calls = 0;
    template<typename StateType, typename To, typename From> bool GuardDummy<StateType, To, From>::CheckReturnValue = true;

    template<typename StateType>
    struct InitialStateNamedFake : StateType {
      static const char* name;
    };
    template<typename StateType> const char* InitialStateNamedFake<StateType>::name = "Initial";

    template<typename T>
    struct FactoryCreatorFake {
      typedef FactoryCreatorFake<T> CreatorType;
      typedef T ObjectType;

      static int createCalls;
      static int deleteCalls;

      static void reset() { createCalls = 0; deleteCalls = 0; }

      static T* create() { createCalls++;  return new T; }
      static void destroy(T* state) { deleteCalls++;  delete state; }
    };
    template<typename T> int FactoryCreatorFake<T>::createCalls = 0;
    template<typename T> int FactoryCreatorFake<T>::deleteCalls = 0;

    template<typename T>
    struct SingletonCreatorFake {
      typedef SingletonCreatorFake<T> CreatorType;
      typedef T ObjectType;

      static int createCalls;
      static int deleteCalls;

      static void reset() { createCalls = 0; deleteCalls = 0; }

      static T* create() { createCalls++;  return instance; }
      static void destroy(T* state) { deleteCalls++;  }
  
    private:
      static T* instance;
    };
    template<typename T> int SingletonCreatorFake<T>::createCalls = 0;
    template<typename T> int SingletonCreatorFake<T>::deleteCalls = 0;
    template<typename T> T* SingletonCreatorFake<T>::instance = new T;
  }
}
