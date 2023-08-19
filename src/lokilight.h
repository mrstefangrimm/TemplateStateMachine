#pragma once
// For the Loki copyright, please visit http://loki-lib.sourceforge.net/
// These types here where either copied from Loki or from the book "Modern C++"

namespace LokiLight {

// From Modern C++
template<bool> struct CompileTimeError;
template<> struct CompileTimeError<true> {};

// From Modern C++
template<uint8_t V>
struct Int2Type {
  enum { value = V };
};

// From Modern C++
class NullType {};

// From Modern C++
template<class T, class U>
struct Typelist {
  typedef T Head;
  typedef U Tail;
};

// From Modern C++
template<class TL, uint8_t INDEX> struct TypeAt;
template<class HEAD, class TAIL>
struct TypeAt<Typelist<HEAD, TAIL>, 0> {
  typedef HEAD Result;
};
template<class HEAD, class TAIL, uint8_t INDEX>
struct TypeAt<Typelist<HEAD, TAIL>, INDEX> {
  typedef typename TypeAt< TAIL, INDEX - 1 >::Result Result;
};

// From Modern C++
template<class TLIST, class T> struct IndexOf;
template<class T>
struct IndexOf<NullType, T> {
  enum { Result = -1 };
};
template<class T, class TAIL>
struct IndexOf<Typelist<T, TAIL>, T> {
  enum { Result = 0 };
};
template<class HEAD, class TAIL, class T>
struct IndexOf<Typelist<HEAD, TAIL>, T> {
private:
  enum { Temp = IndexOf<TAIL, T>::Result };
public:
  enum { Result = Temp == -1 ? -1 : 1 + Temp };
};

// From Modern C++
template<bool flag, class T, class U>
struct Select {
  typedef T Result;
};
template<class T, class U>
struct Select<false, T, U> {
  typedef U Result;
};

// From Loki
template<class TList> struct Length;
template<> struct Length<NullType> {
  enum { value = 0 };
};
template<class T, class U>
struct Length< Typelist<T, U> > {
  enum { value = 1 + Length<U>::value };
};

// From Loki
template<class TList, class T> struct Append;
template<> struct Append<NullType, NullType> {
  typedef NullType Result;
};
template<class T> struct Append<NullType, T> {
  typedef Typelist<T, NullType> Result;
};
template<class Head, class Tail>
struct Append<NullType, Typelist<Head, Tail> > {
  typedef Typelist<Head, Tail> Result;
};
template<class Head, class Tail, class T>
struct Append<Typelist<Head, Tail>, T> {
  typedef Typelist<Head,
                   typename Append<Tail, T>::Result>
    Result;
};
}
