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

#if defined(ARDUINO)

template<class T, class U>
struct is_same {
  enum { value = 0 };
};
template<class T>
struct is_same<T, T> {
  enum { value = 1 };
};

// From stackoverflow: https://stackoverflow.com/questions/2910979/how-does-is-base-of-work
typedef char (&yes)[1];
typedef char (&no)[2];

template<class B, class D>
struct Host {
  operator B*() const;
  operator D*();
};

template<class B, class D>
struct is_base_of {
  template<class T>
  static yes check(D*, T);
  static no check(B*, int);

  static const bool value = sizeof(check(Host<B, D>(), int())) == sizeof(yes);
};

#else

#include <stdint.h>
#include <typeinfo>
#include <type_traits>

using namespace std;

#endif

#include "lokilight.h"
#include "state.h"
#include "statemachine.h"
#include "transition.h"
#include "choicetransition.h"
#include "initialtransition.h"
#include "finaltransition.h"

namespace tsmlib
{
  template<class T, class U>
  using Typelist = LokiLight::Typelist<T, U>;

  using NullType = LokiLight::NullType;
}
