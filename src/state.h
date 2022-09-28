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

// Pre-defined comperators
struct MemoryAddressComperator;
struct VirtualGetTypeIdStateComperator;
struct RttiComperator;

template<typename Comperator, bool Singleton>
struct State {
#ifndef DISABLENESTEDSTATES
  virtual void _vexit() = 0;

  template<uint8_t N> void _exit() { _vexit(); }
#else
  template<uint8_t N> void _exit() {}
#endif

  bool equals(const State<Comperator, Singleton>& other) const {
    return Comperator::areEqual(*this, other);
  }

  template<typename T>
  bool typeOf() {
    return Comperator::template hasType<T>(*this);
  }
};

// Specializations of State class. Non-singletons need some type-id method for comparison.
template<>
struct State<MemoryAddressComperator, true> {
#ifndef DISABLENESTEDSTATES
  virtual void _vexit() = 0;

  template<uint8_t N> void _exit() { _vexit(); }
#else
  template<uint8_t N> void _exit() {}
#endif

  bool equals(const State<MemoryAddressComperator, true>& other) const {
    return this == &other;
  }

  template<typename T>
  bool typeOf() {
    typedef typename T::CreatorType Factory;
    auto other = Factory::create();
    // other is 0 for AnyState
    bool sameType = other != 0 ? this->equals(*other) : true;
    Factory::destroy(other);

    return sameType;
  }
};

template<>
struct State<VirtualGetTypeIdStateComperator, false> {
  virtual uint8_t getTypeId() const = 0;

#ifndef DISABLE_NESTED_STATES
  virtual void _vexit() = 0;
  
  template<uint8_t N> void _exit() { _vexit(); }
#else
  template<uint8_t N> void _exit() {}
#endif

  bool equals(const State<VirtualGetTypeIdStateComperator, false>& other) const {
    auto derived = static_cast<const State<VirtualGetTypeIdStateComperator, false>*>(this);
    return derived->getTypeId() == other.getTypeId();
  }

  template<typename T>
  bool typeOf() {
    typedef typename T::CreatorType Factory;
    auto other = Factory::create();
    // fromState is 0 for AnyState
    bool sameType = this->getTypeId() == other->getTypeId();
    Factory::destroy(other);

    return sameType;
  }
};

#if defined (IAMWORKSTATION)
template<>
struct State<RttiComperator, false> {

#ifndef DISABLE_NESTED_STATES
  virtual void _vexit() = 0;

  template<uint8_t N> void _exit() { _vexit(); }
#else
  template<uint8_t N> void _exit() {}
#endif

  bool equals(const State<RttiComperator, false>& other) const {
    return typeid(*this) == typeid(other);
  }

  template<typename T>
  bool typeOf() {
    return typeid(*this) == typeid(T);
  }
};
#endif

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

}
