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

  template<typename COMPERATOR, typename DERIVED>
  struct StateBase {
    bool equals(const DERIVED& other) const {
      return COMPERATOR::AreEqual(*static_cast<const DERIVED*>(this), other);
    }
    //void exit() {
    //  static_cast<DERIVED*>(this)->Exi();
    //}
    //void entry() {
    //  const DERIVED* obj = static_cast<const DERIVED*>(this);
    //  obj->Entry();
    //}
    //void doit() {
    //  const DERIVED* obj = static_cast<const DERIVED*>(this);
    //  obj->Do();
    //}
  };

  template<typename COMPERATOR, bool MINIMAL>
  struct State { };

  template<typename COMPERATOR>
  struct State<COMPERATOR, false> : StateBase<COMPERATOR, State<COMPERATOR, false>> {
    //Microsoft typeid requires:
    virtual void vvfunc() {}

    // https://stackoverflow.com/questions/54189535/problems-with-implementing-type-id-without-rtti
    virtual uint8_t getTypeId() const = 0;
 
    //bool Equals(const State& other) const {
    //  return COMPERATOR::AreEqual(*this, other);
    //}
  };

  template<typename COMPERATOR>
  struct State<COMPERATOR, true> : StateBase<COMPERATOR, State<COMPERATOR, true>> {

    //bool Equals(const State& other) const {
    //  return COMPERATOR::AreEqual(*this, other);
    //}
  };

  template<typename COMPERATOR, bool MINIMAL>
  inline bool operator==(const State<COMPERATOR, MINIMAL>& lhs, const State<COMPERATOR, MINIMAL>& rhs) {
    return lhs.equals(rhs);
  }

  template<typename T>
  struct SingletonCreator {
    typedef SingletonCreator<T> CreatorType;

    static T* Create() { return Instance; }
    static void Delete(T* state) { }

  private:
    static T* Instance;
  };
  template<typename T> T* SingletonCreator<T>::Instance = new T;

  template<typename T>
  struct FactorCreator {
    typedef FactorCreator<T> CreatorType;

    static T* Create() { return new T; }
    static void Delete(T* state) { delete state; }
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
#endif

  struct VirtualGetTypeIdStateComperator {
    static bool AreEqual(const State<VirtualGetTypeIdStateComperator, false>& lhs, const State<VirtualGetTypeIdStateComperator, false>& rhs) {
      return lhs.getTypeId() == rhs.getTypeId();
    }
  };

  template<bool MINIMAL>
  struct MemoryAddressStateComperator {
    static bool AreEqual(const State<MemoryAddressStateComperator, MINIMAL>& lhs, const State<MemoryAddressStateComperator, MINIMAL>& rhs) {
      return &lhs == &rhs;
    }
  };

}
