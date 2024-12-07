//
// Created by icxd on 10/28/24.
//

#pragma once

#include <LibCpp/type_traits.hpp>

namespace Core {

template <class T, T v>
struct IntegralConstant {
  static constexpr T value = v;
  using type = IntegralConstant;
  using value_type = decltype(value);

  constexpr explicit operator value_type() const { return value; }
  constexpr value_type operator()() const { return value; }
};

using TrueType = IntegralConstant<bool, true>;
using FalseType = IntegralConstant<bool, false>;

namespace Detail {

template <class T> auto TestReturnable(int) -> decltype(
void(static_cast<T(*)()>(nullptr)), TrueType{});

template <class> auto TestReturnable(...) -> FalseType;

template <class From, class To> auto TestImplicitlyConvertible(int) -> decltype(
void(std::declval<void (&)(To)>()(std::declval<From>())), TrueType{});

template <class, class> auto TestImplicitlyConvertible(...) -> FalseType;

} // namespace Detail

template <typename T> struct IsInteger : FalseType {};
template <> struct IsInteger<bool> : TrueType {};
template <> struct IsInteger<char> : TrueType {};
template <> struct IsInteger<signed char> : TrueType {};
template <> struct IsInteger<unsigned char> : TrueType {};
template <> struct IsInteger<wchar_t> : TrueType {};
template <> struct IsInteger<char16_t> : TrueType {};
template <> struct IsInteger<char32_t> : TrueType {};
template <> struct IsInteger<short> : TrueType {};
template <> struct IsInteger<unsigned short> : TrueType {};
template <> struct IsInteger<int> : TrueType {};
template <> struct IsInteger<unsigned int> : TrueType {};
template <> struct IsInteger<long> : TrueType {};
template <> struct IsInteger<unsigned long> : TrueType {};
template <> struct IsInteger<long long> : TrueType {};
template <> struct IsInteger<unsigned long long> : TrueType {};
template <typename T>
inline constexpr bool IsInteger_V = IsInteger<T>::value;

template <typename T> struct IsFloatingPoint : FalseType {};
template <> struct IsFloatingPoint<float> : TrueType {};
template <> struct IsFloatingPoint<double> : TrueType {};
template <> struct IsFloatingPoint<long double> : TrueType {};
template <typename T>
inline constexpr bool IsFloatingPoint_V = IsFloatingPoint<T>::value;

template <class T> struct RemoveConstReference_I { using type = T; };
template <class T> struct RemoveConstReference_I<T &> { using type = T; };
template <class T> struct RemoveConstReference_I<T &&> { using type = T; };
template <class T> struct RemoveConstReference_I<const T &> { using type = T; };
template <class T> using RemoveConstReference = typename RemoveConstReference_I<T>::type;

template <class T> struct RemoveReference_I { using type = T; };
template <class T> struct RemoveReference_I<T &> { using type = T; };
template <class T> struct RemoveReference_I<T &&> { using type = T; };
template <class T> using RemoveReference = typename RemoveReference_I<T>::type;

template <class T> struct RemoveConst_I { using type = T; };
template <class T> struct RemoveConst_I<const T> { using type = T; };
template <class T> using RemoveConst = typename RemoveConst_I<T>::type;

template <class T> struct RemoveVolatile_I { using type = T; };
template <class T> struct RemoveVolatile_I<volatile T> { using type = T; };
template <class T> using RemoveVolatile = typename RemoveVolatile_I<T>::type;

template <class T> struct RemoveCV_I { using type = T; };
template <class T> struct RemoveCV_I<const T> { using type = T; };
template <class T> struct RemoveCV_I<volatile T> { using type = T; };
template <class T> struct RemoveCV_I<const volatile T> { using type = T; };
template <class T> using RemoveCV = RemoveCV_I<T>::type;

template <typename A, typename B> struct IsSame_I : FalseType {};
template <typename A> struct IsSame_I<A, A> : TrueType {};

template <class A, class B> concept IsSame = IsSame_I<A, B>::value;

static_assert(IsSame<int, RemoveReference<int &>>);
static_assert(IsSame<int, RemoveReference<int &&>>);
static_assert(IsSame<int, RemoveReference<int>>);

template <class T> using Pure = RemoveConst<RemoveReference<T>>;
template <class T> using UnderlyingType = __underlying_type (T);

template <class T> struct IsVoid : IsSame_I<void, RemoveCV<T>> {};

template <class From, class To> struct IsConvertible;
template <class From> struct IsConvertible<From, void> : TrueType {};
template <class From, class To> struct IsConvertible<From &, To &> : TrueType {};
template <class From, class To> struct IsConvertible : IntegralConstant<bool,
                                                                        (decltype(Detail::TestReturnable<To>(0))::value
                                                                            &&
                                                                                decltype(Detail::TestImplicitlyConvertible<
                                                                                    From,
                                                                                    To>(0))::value) ||
                                                                            (IsVoid<From>::value && IsVoid<To>::value)
> {
};

} // namespace Core
