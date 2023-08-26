#pragma once

#define IAMWORKSTATION 1
#include "../../src/tsm.h"
#include <vector>

namespace UnitTests {
  namespace Helpers {

    // Dummy, Fake and Stub as suggested by Martin Fowler's article on Test Doubles: https://martinfowler.com/bliki/TestDouble.html?ref=net-from-the-mountain

    using namespace Microsoft::VisualStudio::CppUnitTestFramework;
    using namespace tsmlib;

    template<class To, class From>
    struct ActionStub {
      static int calls;

      template<class StateType, class EventType>
      void perform(StateType&, const EventType&) { calls++; }

      void perform() { calls++; }

    };
    template<class To, class From> int ActionStub<To, From>::calls = 0;

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

    template<class To, class From, class Recorder>
    class ActionSpy {
    public:
      static int calls;

      template<class StateType, class EventType>
      void perform(StateType&, const EventType&) {
        calls++;
        ostringstream buf;
        buf << To::name << "<-" << From::name;
        Recorder::add(buf.str());
      }

      void perform() {
        calls++;
        ostringstream buf;
        buf << To::name << "<-" << From::name;
        Recorder::add(buf.str());
      }
    };
    template<class To, class From, class Recorder> int ActionSpy<To, From, Recorder>::calls = 0;

    template<class StateType, class To, class From>
    struct GuardDummy {
      static int calls;
      static bool CheckReturnValue;

      template<class StateType, class EventType>
      bool eval(const StateType& activeState, const EventType& ev) {
        if (!is_same < From, AnyState<StateType>>().value) {
          From* from = From::CreatorType::create();
          Assert::IsTrue(activeState.equals(*from));
          From::CreatorType::destroy(from);
        }
        calls++;
        return CheckReturnValue;
      }
    };
    template<class StateType, class To, class From> int GuardDummy<StateType, To, From>::calls = 0;
    template<class StateType, class To, class From> bool GuardDummy<StateType, To, From>::CheckReturnValue = true;

    template<class StateType>
    struct InitialStateNamedFake : StateType {
      static const char* name;
    };
    template<class StateType> const char* InitialStateNamedFake<StateType>::name = "Initial";

    template<class T>
    struct FactoryCreatorFake {
      using CreatorType = FactoryCreatorFake<T>;
      using ObjectType = T;

      static int createCalls;
      static int deleteCalls;

      static void reset() { createCalls = 0; deleteCalls = 0; }

      static T* create() { createCalls++;  return new T; }
      static void destroy(T* state) { deleteCalls++;  delete state; }
    };
    template<class T> int FactoryCreatorFake<T>::createCalls = 0;
    template<class T> int FactoryCreatorFake<T>::deleteCalls = 0;

    template<class T>
    struct SingletonCreatorFake {
      using CreatorType = SingletonCreatorFake<T>;
      using ObjectType = T;

      static int createCalls;
      static int deleteCalls;

      static void reset() { createCalls = 0; deleteCalls = 0; }

      static T* create() { createCalls++;  return &instance; }
      static void destroy(T* state) { deleteCalls++;  }
  
    private:
      static T instance;
    };
    template<class T> int SingletonCreatorFake<T>::createCalls = 0;
    template<class T> int SingletonCreatorFake<T>::deleteCalls = 0;
    template<class T> T SingletonCreatorFake<T>::instance;
  }
}
