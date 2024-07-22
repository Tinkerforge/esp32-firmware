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
// a new type.  STRONG_INTEGER_TYPEDEF(T, D, X) creates a new type named D
// that operates as a type T. X is additional code to be implemented in this type.

#define STRONG_INTEGER_TYPEDEF(T, D, X)                                         \
struct D                                                                        \
{                                                                               \
    T t;                                                                        \
    constexpr explicit D(const T t_) : t(t_) {};                                \
    D() = default;                                                              \
    constexpr D(const D & t_) : t(t_.t){}                                       \
    D & operator=(const T & rhs) { t = rhs; return *this;}                      \
    explicit operator T () const { return t; }                                  \
    X                                                                           \
};                                                                              \
/* Assignment operators */                                                      \
inline D& operator+=(D& x, const D& y) { x.t += y.t; return x;}                 \
inline D& operator-=(D& x, const D& y) { x.t -= y.t; return x;}                 \
inline D& operator*=(D& x, const D& y) { x.t *= y.t; return x;}                 \
inline D& operator/=(D& x, const D& y) { x.t /= y.t; return x;}                 \
inline D& operator%=(D& x, const D& y) { x.t %= y.t; return x;}                 \
inline D& operator&=(D& x, const D& y) { x.t &= y.t; return x;}                 \
inline D& operator|=(D& x, const D& y) { x.t |= y.t; return x;}                 \
inline D& operator^=(D& x, const D& y) { x.t ^= y.t; return x;}                 \
inline D& operator<<=(D& x, const D& y) { x.t <<= y.t; return x;}               \
inline D& operator>>=(D& x, const D& y) { x.t >>= y.t; return x;}               \
/* Increment/decrement operators */                                             \
inline D& operator++(D& x) { ++x.t; return x; }                                 \
inline D& operator--(D& x) { --x.t; return x; }                                 \
inline D operator++(D& x, int a) { return D{x.t++}; }                           \
inline D operator--(D& x, int a) { return D{x.t--}; }                           \
/* Arithmetic operators */                                                      \
constexpr inline D operator+(const D& x) { return D{+x.t}; }                    \
constexpr inline D operator-(const D& x) { return D{-x.t}; }                    \
constexpr inline D operator+(const D& x, const D& y) { return D{x.t + y.t}; }   \
constexpr inline D operator-(const D& x, const D& y) { return D{x.t - y.t}; }   \
constexpr inline D operator*(const D& x, const D& y) { return D{x.t * y.t}; }   \
constexpr inline D operator/(const D& x, const D& y) { return D{x.t / y.t}; }   \
constexpr inline D operator%(const D& x, const D& y) { return D{x.t % y.t}; }   \
constexpr inline D operator~(const D& x) { return D{~x.t}; }                    \
constexpr inline D operator&(const D& x, const D& y) { return D{x.t & y.t}; }   \
constexpr inline D operator|(const D& x, const D& y) { return D{x.t | y.t}; }   \
constexpr inline D operator^(const D& x, const D& y) { return D{x.t ^ y.t}; }   \
constexpr inline D operator<<(const D& x, const D& y) { return D{x.t << y.t}; } \
constexpr inline D operator>>(const D& x, const D& y) { return D{x.t >> y.t}; } \
/* Logical operators */                                                         \
constexpr inline bool operator!(const D& x) { return !x.t; }                    \
constexpr inline bool operator&&(const D& x, const D& y) { return x.t && y.t; } \
constexpr inline bool operator||(const D& x, const D& y) { return x.t || y.t; } \
/* Comparison operators */                                                      \
constexpr inline bool operator==(const D& x, const D& y) { return x.t == y.t; } \
constexpr inline bool operator!=(const D& x, const D& y) { return x.t != y.t; } \
constexpr inline bool operator<(const D& x, const D& y) { return x.t < y.t; }   \
constexpr inline bool operator>(const D& x, const D& y) { return x.t > y.t; }   \
constexpr inline bool operator<=(const D& x, const D& y) { return x.t <= y.t; } \
constexpr inline bool operator>=(const D& x, const D& y) { return x.t >= y.t; }
