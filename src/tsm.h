#pragma once

#if defined(__AVR__)

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

using namespace std;

#endif

#include "lokilight.h"
#include "state.h"
#include "statemachine.h"
#include "transition.h"
#include "choicetransition.h"
#include "initialtransition.h"
#include "finaltransition.h"
