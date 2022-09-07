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

namespace tsmlib {

template<typename Derived, typename Comperator>
struct StateBase {
  bool equals(const Derived& other) const {
    return Comperator::AreEqual(*static_cast<const Derived*>(this), other);
  }
};

template<typename Comperator, bool Singleton>
struct State {
  // TODO: #ifndef SARCRIFYEXIT
  //  virtual void exit() = 0;
  //#endif

  bool equals(const State& other) const {
    return Comperator::AreEqual(*this, other);
  }
};

template<typename Comperator>
struct State<Comperator, false> : StateBase<State<Comperator, false>, Comperator> {
  //Microsoft typeid requires:
  virtual void vvfunc() {}

  virtual uint8_t getTypeId() const = 0;
};

template<typename Derived, typename Basetype>
struct SimpleState : Basetype {

  void entry() {
    static_cast<Derived*>(this)->entry_();
  }
  void exit() {
    static_cast<Derived*>(this)->exit_();
  }

  template<uint8_t N>
  void doit() {
    static_cast<Derived*>(this)->template doit_<N>();
  }
};

template<typename Derived, typename Basetype, typename Statemachine>
struct StatemachineHolderState : Basetype {

  void entry() {
    static_cast<Derived*>(this)->entry_();
    _nestedSM.begin();
  }
  void exit() {
    _nestedSM.end();
    static_cast<Derived*>(this)->exit_();
  }

  template<uint8_t N>
  void doit() {

    // Check if nested consumes the trigger
    if (_nestedSM.template trigger<N>().consumed) return;

    static_cast<Derived*>(this)->template doit_<N>();
  }

  Statemachine _nestedSM;
};

template<typename COMPERATOR, bool MINIMAL>
bool operator==(const State<COMPERATOR, MINIMAL>& lhs, const State<COMPERATOR, MINIMAL>& rhs) {
  return lhs.equals(rhs);
}

template<typename T>
struct SingletonCreator {
    typedef SingletonCreator<T> CreatorType;
    typedef T ObjectType;

    static T* Create() {
      return Instance;
    }
    static void Delete(T* state) { }

  private:
    static T* Instance;
};
template<typename T> T* SingletonCreator<T>::Instance = new T;

template<typename T>
struct FactorCreator {
  typedef FactorCreator<T> CreatorType;
  typedef T ObjectType;

  static T* Create() {
    return new T;
  }
  static void Delete(T* state) {
    delete state;
  }
};

#ifndef IAMARDUINO
struct TypeidStateComperator {
  static bool AreEqual(const State<TypeidStateComperator, false>& lhs, const State<TypeidStateComperator, false>& rhs) {
    // TODO: doesn't work with base and derived class
    const type_info& l = typeid(lhs);
    const type_info& r = typeid(rhs);
    bool ret = (l == r);
    return ret;
  }
};

struct VirtualGetTypeIdStateComperator {
  static bool AreEqual(const State<VirtualGetTypeIdStateComperator, false>& lhs, const State<VirtualGetTypeIdStateComperator, false>& rhs) {
    return lhs.getTypeId() == rhs.getTypeId();
  }
};

#endif

template<bool MINIMAL>
struct MemoryAddressStateComperator {
  static bool AreEqual(const State<MemoryAddressStateComperator, MINIMAL>& lhs, const State<MemoryAddressStateComperator, MINIMAL>& rhs) {
    return &lhs == &rhs;
  }
};
}
