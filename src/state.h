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
    using Factory = typename T::CreatorType;
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
    return this->getTypeId() == other.getTypeId();
  }

  template<class T>
  bool typeOf() const {
    using Factory = typename T::CreatorType;
    T* other = Factory::create();
    // other is nullptr for AnyState. Rule: AnyState != AnyState
    if (other == nullptr) return false;

    bool sameType = this->getTypeId() == other->getTypeId();
    Factory::destroy(other);

    return sameType;
  }
};

#if !defined(__AVR__)
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
  using CreatorType = AnyState<T>;
  using ObjectType = AnyState<T>;

  static AnyState* create() {
    return nullptr;
  }
  static void destroy(AnyState*) {}
};

template<class Derived, class Basetype, bool HasEntry = false, bool HasExit = false, bool HasDoit = false>
class BasicState : public Basetype {
public:
  enum { BasicDoit = HasDoit };

  template<class Event>
  void _entry(const Event& ev) {
    __entry(ev, Int2Type<HasEntry>());
  }

  template<class Event>
  void _exit(const Event& ev) {
    __exit(ev, Int2Type<HasExit>());
  }

  template<class Event>
  bool _doit(const Event& ev) {
    __doit(ev, Int2Type<HasDoit>());
    return true;
  }

private:
  template<class Event>
  void __entry(const Event& ev, const Int2Type<false>&) {
  }
  template<class Event>
  void __entry(const Event& ev, const Int2Type<true>&) {
    static_cast<Derived*>(this)->template entry<Event>(ev);
  }
  template<class Event>
  void __exit(const Event& ev, const Int2Type<false>&) {
  }
  template<class Event>
  void __exit(const Event& ev, const Int2Type<true>&) {
    static_cast<Derived*>(this)->template exit<Event>(ev);
  }
  template<class Event>
  void __doit(const Event& ev, const Int2Type<false>&) {
  }
  template<class Event>
  void __doit(const Event& ev, const Int2Type<true>&) {
    static_cast<Derived*>(this)->template doit<Event>(ev);
  }
};

template<class Derived, class Basetype, class Statemachine, bool HasEntry = false, bool HasExit = false>
class SubstatesHolderState : public Basetype {
public:
  enum { BasicDoit = false };

  template<class Event>
  void _entry(const Event& ev) {
    __entry(ev, Int2Type<HasExit>());
    subStatemachine_.template _begin<Event>();
  }

  template<class Event>
  void _exit(const Event& ev) {
    subStatemachine_.template _end<Event>();
    __exit(ev, Int2Type<HasExit>());
  }

  template<class Event>
  bool _doit(const Event& ev) {
    auto result = subStatemachine_.template dispatch<Event>(ev);
    return result.consumed;
  }

private:
  template<class Event>
  void __entry(const Event& ev, const Int2Type<false>&) {
  }
  template<class Event>
  void __entry(const Event& ev, const Int2Type<true>&) {
    static_cast<Derived*>(this)->template entry<Event>(ev);
  }
  template<class Event>
  void __exit(const Event& ev, const Int2Type<false>&) {
  }
  template<class Event>
  void __exit(const Event& ev, const Int2Type<true>&) {
    static_cast<Derived*>(this)->template exit<Event>(ev);
  }

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
  using CreatorType = SingletonCreator<T>;
  using ObjectType = T;

  static T* create() {
    return &instance;
  }
  static void destroy(T*) {}

private:
  static T instance;
};
template<class T> T SingletonCreator<T>::instance;

/**
* When leaving, the object is destroyed and the state's state is lost.
*/
template<class T, bool implementsCreate = true>
struct FactoryCreator {
  using CreatorType = FactoryCreator<T, true>;
  using ObjectType = T;

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
  using CreatorType = FactoryCreator<T, false>;
  using ObjectType = T;

  static void* create() {
    CompileTimeError<true>();
  }
  static void destroy(T* state) {
    delete state;
  }
};

}
