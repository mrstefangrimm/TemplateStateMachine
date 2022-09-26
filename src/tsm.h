#pragma once

#if defined(IAMARDUINO)

template<class T, class U>
struct is_same {
  enum { value = 0 };
};
template<class T>
struct is_same<T, T> {
  enum { value = 1 };
};

#define ISMINIMAL true

#elif defined (IAMWORKSTATION)

#include <stdint.h>
#include <typeinfo>

using namespace std;

#else
#error define either IAMARDUINO or IAMWORKSTATION
#endif

#include "templatemeta.h"
#include "state.h"
#include "statemachine.h"
#include "transition.h"
#include "choicetransition.h"
#include "initialtransition.h"
#include "endtransition.h"
