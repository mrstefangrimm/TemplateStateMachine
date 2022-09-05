#pragma once

#if defined(IAMARDINO)

template<class T, class U>
struct is_same {
  enum { value = 0 };
};
template<class T>
struct is_same<T, T> {
  enum { value = 1 };
};

#define ISMINIMAL true

#elif defined (IAMWINDOWS)

#include <stdint.h>
#include <typeinfo>

#define ISMINIMAL false

using namespace std;

#else
#error define either IAMARDINO or IAMWINDOWS
#endif

#include "templatemeta.h"
#include "state.h"
#include "statemachine.h"
#include "transition.h"
