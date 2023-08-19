#pragma once
/*
  Copyright 2022-2023 Stefan Grimm

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

#include "lokilight.h"

namespace tsmlib {

using namespace LokiLight;
// Pre-defined comparators
struct MemoryAddressComparator;
struct VirtualGetTypeIdStateComparator;
struct RttiComparator;

template<class Comparator, bool Singleton>
struct State {

  bool equals(const State& other) const {
    return Comparator::areEqual(*this, other);
  }

  template<class T>
  bool typeOf() const {
    return Comparator::template hasType<T>(*this);
  }
};
// Specializations of State class. Non-singletons need some type-id method for comparison.
template<>
struct State<MemoryAddressComparator, true> {

  bool equals(const State<MemoryAddressComparator, true>& other) const {
    return this == &other;
  }

  template<class T>
  bool typeOf() const {
    typedef typename T::CreatorType Factory;
    T* other = Factory::create();
    // other is 0 for AnyState
    bool sameType = other != 0 ? this->equals(*other) : true;
    Factory::destroy(other);

    return sameType;
  }
};

template<>
struct State<VirtualGetTypeIdStateComparator, false> {

  virtual uint8_t getTypeId() const = 0;

  bool equals(const State<VirtualGetTypeIdStateComparator, false>& other) const {
    auto derived = static_cast<const State<VirtualGetTypeIdStateComparator, false>*>(this);
    return derived->getTypeId() == other.getTypeId();
  }

  template<class T>
  bool typeOf() const {
    typedef typename T::CreatorType Factory;
    T* other = Factory::create();
    // other is nullptr for AnyState. Rule: AnyState != AnyState
    if (other == nullptr) return false;

    bool sameType = this->getTypeId() == other->getTypeId();
    Factory::destroy(other);

    return sameType;
  }
};

#if defined(IAMWORKSTATION)
template<>
struct State<RttiComparator, false> {
public:
  // Google: Why does C++ RTTI require a virtual method table?
  virtual ~State() {}

  bool equals(const State<RttiComparator, false>& other) const {
    return typeid(*this) == typeid(other);
  }

  template<class T>
  bool typeOf() const {
    return typeid(*this) == typeid(T);
  }
};
#endif

template<class T>
struct AnyState : T {
  typedef AnyState CreatorType;
  typedef AnyState ObjectType;

  static AnyState* create() {
    return nullptr;
  }
  static void destroy(AnyState*) {}
};

template<class Derived, class Basetype>
class BasicState : public Basetype {
public:
  template<class Event>
  void _entry() {
    static_cast<Derived*>(this)->entry();
  }

  template<class event>
  void _exit() {
    static_cast<Derived*>(this)->exit();
  }

  template<class Event>
  bool _doit() {
    static_cast<Derived*>(this)->template doit<Event>();
    return true;
  }
};

template<class Derived, class Basetype, class Statemachine>
class SubstatesHolderState : public Basetype {
public:
  template<class Event>
  void _entry() {
    static_cast<Derived*>(this)->entry();
    subStatemachine_.template _begin<Event>();
  }

  template<class Event>
  void _exit() {
    subStatemachine_.template _end<Event>();
    static_cast<Derived*>(this)->exit();
  }

  template<class Event>
  bool _doit() {
    auto result = subStatemachine_.template dispatch<Event>();
    return result.consumed;
  }

private:
  Statemachine subStatemachine_;
};

template<class Comparator, bool Singleton>
bool operator==(const State<Comparator, Singleton>& lhs, const State<Comparator, Singleton>& rhs) {
  return lhs.equals(rhs);
}

/**
* When leaving, the object not destroyed. Make sure you reset the state's state.
*/
template<class T>
struct SingletonCreator {
  typedef SingletonCreator<T> CreatorType;
  typedef T ObjectType;

  static T* create() {
    return instance;
  }
  static void destroy(T* state) {}

private:
  static T* instance;
};
template<class T> T* SingletonCreator<T>::instance = new T;

/**
* When leaving, the object is destroyed and the state's state is lost.
*/
template<class T, bool implementsCreate = true>
struct FactoryCreator {
  typedef FactoryCreator<T, true> CreatorType;
  typedef T ObjectType;

  static T* create() {
    return new T;
  }
  static void destroy(T* state) {
    delete state;
  }
};
// Specialization
template<class T>
struct FactoryCreator<T, false> {
  typedef FactoryCreator<T, false> CreatorType;
  typedef T ObjectType;

  static void* create() {
    CompileTimeError<true>();
  }
  static void destroy(T* state) {
    delete state;
  }
};

}
