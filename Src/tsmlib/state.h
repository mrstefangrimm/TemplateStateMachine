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
    return Comperator::areEqual(*static_cast<const Derived*>(this), other);
  }
};

template<typename Comperator, bool Singleton>
struct State {
  bool equals(const State& other) const {
    return Comperator::areEqual(*this, other);
  }
#ifndef DISABLENESTEDSTATES
  virtual void exit() = 0;
#else
  void exit() {}
#endif
};
// specialization of State class.
template<typename Comperator>
struct State<Comperator, false> : StateBase<State<Comperator, false>, Comperator> {
  //Microsoft typeid requires:
  virtual void vvfunc() {}

  virtual uint8_t getTypeId() const = 0;
};

template<typename State>
struct AnyState : State {
  typedef AnyState CreatorType;
  //typedef AnyState ObjectType;

  static AnyState* create() {
    return 0;
  }
  static void destroy(AnyState*) { }
};

template<typename Derived, typename Basetype>
class SimpleState : public Basetype {
  public:
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
class SubstatesHolderState : public Basetype {
  public:
    void entry() {
      static_cast<Derived*>(this)->entry_();
      _subStatemachine.begin();
    }

    void exit() {
      _subStatemachine.end();
      static_cast<Derived*>(this)->exit_();
    }

    template<uint8_t N>
    void doit() {

      // Return if substates consumed the trigger
      if (_subStatemachine.template dispatch<N>().consumed) return;

      static_cast<Derived*>(this)->template doit_<N>();
    }

    Statemachine _subStatemachine;
};

template<typename Comperator, bool Minimal>
bool operator==(const State<Comperator, Minimal>& lhs, const State<Comperator, Minimal>& rhs) {
  return lhs.equals(rhs);
}

template<typename T>
struct SingletonCreator {
    typedef SingletonCreator<T> CreatorType;
    //typedef T ObjectType;

    static T* create() {
      return Instance;
    }
    static void destroy(T* state) { }

  private:
    static T* Instance;
};
template<typename T> T* SingletonCreator<T>::Instance = new T;

template<typename T>
struct FactorCreator {
  typedef FactorCreator<T> CreatorType;
  //typedef T ObjectType;

  static T* create() {
    return new T;
  }
  static void destroy(T* state) {
    delete state;
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
};

struct VirtualGetTypeIdStateComperator {
  static bool areEqual(const State<VirtualGetTypeIdStateComperator, false>& lhs, const State<VirtualGetTypeIdStateComperator, false>& rhs) {
    return lhs.getTypeId() == rhs.getTypeId();
  }
};
#endif

template<bool Minimal>
struct MemoryAddressStateComperator {
  static bool areEqual(const State<MemoryAddressStateComperator, Minimal>& lhs, const State<MemoryAddressStateComperator, Minimal>& rhs) {
    return &lhs == &rhs;
  }
};
}
