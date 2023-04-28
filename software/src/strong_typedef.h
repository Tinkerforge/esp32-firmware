#pragma once

// Extracted and modified from boost 1.82.
// strong_typedef.hpp:

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org/libs/serialization for updates, documentation, and revision history.

// macro used to implement a strong typedef.  strong typedef
// guarentees that two types are distinguised even though the
// share the same underlying implementation.  typedef does not create
// a new type.  STRONG_INTEGER_TYPEDEF(T, D) creates a new type named D
// that operates as a type T.

#define STRONG_INTEGER_TYPEDEF(T, D)                                    \
struct D                                                                \
{                                                                       \
    T t;                                                                \
    explicit D(const T t_) : t(t_) {};                                  \
    D(){};                                                              \
    D(const D & t_) : t(t_.t){}                                         \
    D & operator=(const D & rhs) { t = rhs.t; return *this;}            \
    D & operator=(const T & rhs) { t = rhs; return *this;}              \
    explicit operator const T & () const { return t; }                  \
    explicit operator T & () { return t; }                              \
};                                                                      \
/* Assignment operators */                                              \
inline D& operator+=(D& x, const D& y) { x.t += y.t; return x;}         \
inline D& operator-=(D& x, const D& y) { x.t -= y.t; return x;}         \
inline D& operator*=(D& x, const D& y) { x.t *= y.t; return x;}         \
inline D& operator/=(D& x, const D& y) { x.t /= y.t; return x;}         \
inline D& operator%=(D& x, const D& y) { x.t %= y.t; return x;}         \
inline D& operator&=(D& x, const D& y) { x.t &= y.t; return x;}         \
inline D& operator|=(D& x, const D& y) { x.t |= y.t; return x;}         \
inline D& operator^=(D& x, const D& y) { x.t ^= y.t; return x;}         \
inline D& operator<<=(D& x, const D& y) { x.t <<= y.t; return x;}       \
inline D& operator>>=(D& x, const D& y) { x.t >>= y.t; return x;}       \
/* Increment/decrement operators */                                     \
inline D& operator++(D& x) { ++x.t; return x; }                         \
inline D& operator--(D& x) { --x.t; return x; }                         \
inline D operator++(D& x, int a) { return D{x.t++}; }                   \
inline D operator--(D& x, int a) { return D{x.t--}; }                   \
/* Arithmetic operators */                                              \
inline D operator+(const D& x) { return D{+x.t}; }                      \
inline D operator-(const D& x) { return D{-x.t}; }                      \
inline D operator+(const D& x, const D& y) { return D{x.t + y.t}; }     \
inline D operator-(const D& x, const D& y) { return D{x.t - y.t}; }     \
inline D operator*(const D& x, const D& y) { return D{x.t * y.t}; }     \
inline D operator/(const D& x, const D& y) { return D{x.t / y.t}; }     \
inline D operator%(const D& x, const D& y) { return D{x.t % y.t}; }     \
inline D operator~(const D& x) { return D{~x.t}; }                      \
inline D operator&(const D& x, const D& y) { return D{x.t & y.t}; }     \
inline D operator|(const D& x, const D& y) { return D{x.t | y.t}; }     \
inline D operator^(const D& x, const D& y) { return D{x.t ^ y.t}; }     \
inline D operator<<(const D& x, const D& y) { return D{x.t << y.t}; }   \
inline D operator>>(const D& x, const D& y) { return D{x.t >> y.t}; }   \
/* Logical operators */                                                 \
inline bool operator!(const D& x) { return !x.t; }                      \
inline bool operator&&(const D& x, const D& y) { return x.t && y.t; }   \
inline bool operator||(const D& x, const D& y) { return x.t || y.t; }   \
/* Comparison operators */                                              \
inline bool operator==(const D& x, const D& y) { return x.t == y.t; }   \
inline bool operator!=(const D& x, const D& y) { return x.t != y.t; }   \
inline bool operator<(const D& x, const D& y) { return x.t < y.t; }     \
inline bool operator>(const D& x, const D& y) { return x.t > y.t; }     \
inline bool operator<=(const D& x, const D& y) { return x.t <= y.t; }   \
inline bool operator>=(const D& x, const D& y) { return x.t >= y.t; }
