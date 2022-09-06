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

// From Modern C++
template<bool> struct CTAssert;
template<> struct CTAssert<true> {};

// From Modern C++
template<uint8_t V>
struct Int2Type {
  enum { value = V };
};

// From Modern C++
class NullType { };

// From Modern C++
template<class T, typename U>
struct Typelist {
  typedef T Head;
  typedef U Tail;
};

// From Modern C++
template<class TL, uint8_t INDEX> struct TypeAt;
template<class HEAD, class TAIL>
struct TypeAt<Typelist<HEAD, TAIL>, 0>
{
  typedef HEAD Result;
};
template<class HEAD, class TAIL, uint8_t INDEX>
struct TypeAt<Typelist<HEAD, TAIL>, INDEX>
{
  typedef typename TypeAt < TAIL, INDEX - 1 >::Result Result;
};

// From Modern C++
template<class TLIST, class T> struct IndexOf;
template<class T>
struct IndexOf<NullType, T>
{
  enum { Result = -1 };
};
template<class T, class TAIL>
struct IndexOf<Typelist<T, TAIL>, T>
{
  enum { Result = 0 };
};
template<class HEAD, class TAIL, class T>
struct IndexOf<Typelist<HEAD, TAIL>, T>
{
  private:
    enum { Temp = IndexOf<TAIL, T>::Result };
  public:
    enum { Result = Temp == -1 ? -1 : 1 + Temp };
};
}
