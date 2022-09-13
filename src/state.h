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
  virtual void _exit() = 0;
#else
  void _exit() {}
#endif
};
// specialization of State class.
template<typename Comperator>
struct State<Comperator, false> : _StateBase<State<Comperator, false>, Comperator> {
  //Microsoft typeid requires:
  virtual void vvfunc() {}
  virtual uint8_t getTypeId() const = 0;

#ifndef DISABLE_NESTED_STATES
  virtual void _exit() = 0;
#else
  void _exit() {}
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
class SimpleState : public Basetype {
  public:
    template<uint8_t N>
    bool _entry() {
      static_cast<Derived*>(this)->entry();
      return false;
    }

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

    void _exit() {
      subStatemachine_.end();
      static_cast<Derived*>(this)->exit();
    }

    template<uint8_t N>
    Basetype* _doit() {

      // Return if substates consumed the trigger
      auto result = subStatemachine_.template dispatch<N>();
      if (result.consumed && result.deferredEntry) {
        return result.activeState;
      }
      // TODO: if consumed and ??? 
      if (result.consumed && false) {
        static_cast<Derived*>(this)->entry();
      }
      static_cast<Derived*>(this)->template doit<N>();
      return 0;
    }

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
      return Instance;
    }
    static void destroy(T* state) { }

  private:
    static T* Instance;
};
template<typename T> T* SingletonCreator<T>::Instance = new T;

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

#if defined (IAMWINDOWS)
struct TypeidStateComperator {
  static bool areEqual(const State<TypeidStateComperator, false>& lhs, const State<TypeidStateComperator, false>& rhs) {
    // TODO: doesn't work with base and derived class
    const type_info& l = typeid(lhs);
    const type_info& r = typeid(rhs);
    bool ret = (l == r);
    return ret;
  }

  template<typename T>
  static bool hasType(const State<TypeidStateComperator, false>* me) {
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
#endif

}
