//
// Created by icxd on 10/29/24.
//


#pragma once

namespace std {

namespace detail {

template <class T> struct type_identity { using type = T; };
template <class T> auto try_add_lvalue_reference(int) -> type_identity<T &>;
template <class T> auto try_add_lvalue_reference(...) -> type_identity<T>;

template <class T> auto try_add_rvalue_reference(int) -> type_identity<T &&>;
template <class T> auto try_add_rvalue_reference(...) -> type_identity<T>;

} // namespace detail

template <class T> struct add_lvalue_reference : decltype(detail::try_add_lvalue_reference<T>(0)) { };
template <class T> struct add_rvalue_reference : decltype(detail::try_add_rvalue_reference<T>(0)) { };

template <typename T> typename std::add_rvalue_reference<T>::type declval() noexcept {
  static_assert(false, "declval not allowed in an evaluated context");
}

template <typename T> struct is_enum { static constexpr bool value = __is_enum(T); };
template <typename T> inline constexpr bool is_enum_v = is_enum<T>::value;

template <typename T> struct underlying_type { using type = __underlying_type(T); };
template <typename T> using underlying_type_t = typename underlying_type<T>::type;

template <typename Enum>
constexpr auto to_underlying(Enum e) noexcept {
  static_assert(is_enum_v<Enum>, "std::to_underlying requires an enum type");
  return static_cast<underlying_type_t<Enum>>(e);
}

}
