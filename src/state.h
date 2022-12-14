#pragma once
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

#include "templatemeta.h"

namespace tsmlib {

using namespace Loki;

template<typename Derived, typename Comperator>
struct _StateBase {
  bool equals(const Derived& other) const {
    return Comperator::areEqual(*static_cast<const Derived*>(this), other);
  }

  template<typename T>
  bool typeOf() {
    return Comperator::template hasType<T>(*static_cast<const Derived*>(this));
  }
};

#if defined (IAMWORKSTATION)

// TODO: Typeid only works with `virtual void vvfunc() {}` with the Microsoft compiler.

template<typename Derived>
struct _StateBase<Derived, struct TypeidStateComperator> {
    bool equals(const Derived& other) const {
      auto derived = dynamic_cast<const Derived*>(this);
      return typeid(*derived) == typeid(other);
    }

    template<typename T>
    bool typeOf() {
      auto derived = dynamic_cast<const Derived*>(this);
      return typeid(*derived) == typeid(T);
    }
    //Microsoft typeid requires:
    virtual void vvfunc() {}
  };

#endif

template<typename Comperator, bool Singleton>
struct State {
  bool equals(const State& other) const {
    return Comperator::areEqual(*this, other);
  }
  template<typename T>
  bool typeOf() {
    return Comperator::template hasType<T>(this);
  }
#ifndef DISABLENESTEDSTATES
  virtual void _vexit() = 0;

  template<uint8_t N> void _exit() { _vexit(); }
#else
  template<uint8_t N> void _exit() {}
#endif
};
// specialization of State class.
template<typename Comperator>
struct State<Comperator, false> : _StateBase<State<Comperator, false>, Comperator> {
  virtual uint8_t getTypeId() const = 0;

#ifndef DISABLE_NESTED_STATES
  virtual void _vexit() = 0;
  
  template<uint8_t N> void _exit() { _vexit(); }
#else
  template<uint8_t N> void _exit() {}
#endif
};

template<typename T>
struct AnyState : T {
  typedef AnyState CreatorType;
  typedef AnyState ObjectType;

  static AnyState* create() {
    return 0;
  }
  static void destroy(AnyState*) { }
};

template<typename Derived, typename Basetype>
class BasicState : public Basetype {
  public:
    template<uint8_t N>
    bool _entry() {
      static_cast<Derived*>(this)->entry();
      return false;
    }

    void _vexit() {
      static_cast<Derived*>(this)->exit();
    }

    template<uint8_t N>
    void _exit() {
      static_cast<Derived*>(this)->exit();
    }

    template<uint8_t N>
    Basetype* _doit() {
      static_cast<Derived*>(this)->template doit<N>();
      return 0;
    }
};

#ifndef DISABLENESTEDSTATES

template<typename Derived, typename Basetype, typename Statemachine>
class SubstatesHolderState : public Basetype {
  public:
    template<uint8_t N>
    bool _entry() {
      static_cast<Derived*>(this)->entry();
      subStatemachine_.template _begin<N>();
      return true;
    }

    void _vexit() {
      static_cast<Derived*>(this)->exit();
    }

    template<uint8_t N>
    void _exit() {
      subStatemachine_.template _end<N>();
      static_cast<Derived*>(this)->exit();
    }

    template<uint8_t N>
    Basetype* _doit() {
      // Return if substates consumed the trigger
      auto result = subStatemachine_.template dispatch<N>();
      if (result.consumed && result.deferredEntry) {
        return result.activeState;
      }
      return 0;
    }

private:
    Statemachine subStatemachine_;
};

#endif

template<typename Comperator, bool Singleton>
bool operator==(const State<Comperator, Singleton>& lhs, const State<Comperator, Singleton>& rhs) {
  return lhs.equals(rhs);
}

template<typename T>
struct SingletonCreator {
    typedef SingletonCreator<T> CreatorType;
    typedef T ObjectType;

    static T* create() {
      return instance;
    }
    static void destroy(T* state) { }

  private:
    static T* instance;
};
template<typename T> T* SingletonCreator<T>::instance = new T;

template<typename T, bool implementsCreate = true>
struct FactorCreator {
  typedef FactorCreator<T, true> CreatorType;
  typedef T ObjectType;

  static T* create() {
    return new T;
  }
  static void destroy(T* state) {
    delete state;
  }
};
// Specialication
template<typename T>
struct FactorCreator<T, false> {
  typedef FactorCreator<T, false> CreatorType;
  typedef T ObjectType;

  static void* create() {
    CompileTimeError<true>();
  }
  static void destroy(T* state) {
    delete state;
  }
};

template<bool Singleton>
struct MemoryAddressStateComperator {
  static bool areEqual(const State<MemoryAddressStateComperator, Singleton>& lhs, const State<MemoryAddressStateComperator, Singleton>& rhs) {
    return &lhs == &rhs;
  }

  template<typename T>
  static bool hasType(const State<MemoryAddressStateComperator, Singleton>* me) {
    typedef typename T::CreatorType Factory;
    auto other = Factory::create();
    // fromState is 0 for AnyState
    bool sameType = other != 0 ? me->equals(*other) : true;
    Factory::destroy(other);

    return sameType;
  }
};

struct VirtualGetTypeIdStateComperator {
  static bool areEqual(const State<VirtualGetTypeIdStateComperator, false>& lhs, const State<VirtualGetTypeIdStateComperator, false>& rhs) {
    return lhs.getTypeId() == rhs.getTypeId();
  }

  template<typename T>
  static bool hasType(const State<VirtualGetTypeIdStateComperator, false>& me) {
    typedef typename T::CreatorType Factory;
    auto other = Factory::create();
    // fromState is 0 for AnyState
    bool sameType = other != 0 ? me.equals(*other) : true;
    Factory::destroy(other);

    return sameType;
  }
};

}
