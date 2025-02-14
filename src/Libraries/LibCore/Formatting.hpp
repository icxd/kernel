//
// Created by icxd on 11/1/24.
//

#pragma once

#include "FormatTypes.hpp"
#include "String.hpp"
#include "Trait.hpp"
#include "Types.hpp"

namespace Core {

  template <typename> struct Formatter {};

  template <typename T>
  concept Formattable = requires(T t) { Formatter<T>::format(t); };

#define CORE_BASIC_FORMATTER(type)                                             \
  template <> struct Formatter<type> {                                         \
    static String format(type value) {                                         \
      return StringBuilder().append(value).build();                            \
    }                                                                          \
  }

  CORE_BASIC_FORMATTER(i8);
  CORE_BASIC_FORMATTER(i16);
  CORE_BASIC_FORMATTER(i32);
  CORE_BASIC_FORMATTER(i64);
  // CORE_BASIC_FORMATTER(isz);
  CORE_BASIC_FORMATTER(u8);
  CORE_BASIC_FORMATTER(u16);
  CORE_BASIC_FORMATTER(u32);
  CORE_BASIC_FORMATTER(u64);
  // CORE_BASIC_FORMATTER(usz);
  CORE_BASIC_FORMATTER(float);
  CORE_BASIC_FORMATTER(double);
  CORE_BASIC_FORMATTER(char);
  CORE_BASIC_FORMATTER(char *);
  CORE_BASIC_FORMATTER(const char *);
  CORE_BASIC_FORMATTER(String);
  CORE_BASIC_FORMATTER(bool);

#undef CORE_BASIC_FORMATTER

  template <typename T> struct Formatter<T *> {
    static String format(T *value) {
      return StringBuilder().append("0x").append((void *)value).build();
    }
  };

  template <typename T>
    requires(Formattable<T>)
  void format_helper(StringBuilder &builder, String &fmt, const T &value) {
    const isz open_brace = fmt.find('{');
    if (open_brace == -1)
      return;

    const isz colon = fmt.find(':', open_brace + 1);
    if (colon == -1) {
      const isz close_brace = fmt.find('}', open_brace + 1);
      if (close_brace == -1)
        return;

      builder.append(fmt.substring(0, open_brace))
          .append(
              Formatter<RemoveConstReference<decltype(value)>>::format(value));
      fmt = fmt.substring(close_brace + 1);

      return;
    }

    // format_spec     ::=
    // [[fill]align][sign]["z"]["#"]["0"][width][grouping_option]["."
    // precision][type] fill            ::=  <any character> align           ::=
    // "<" | ">" | "=" | "^" sign            ::=  "+" | "-" | " " width ::=
    // digit+ grouping_option ::=  "_" | "," precision       ::=  digit+ type
    // ::=  "b" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G" | "n" | "o" |
    // "s" | "x" | "X" | "%"

    const isz close_brace = fmt.find('}', open_brace + 1);
    if (close_brace == -1)
      return;
    builder.append(fmt.substring(0, open_brace));

    auto integer_repr = FormatterIntegerRepresentation::None;
    auto floating_repr = FormatterFloatingRepresentation::None;

    const char type_char = fmt.at(colon + 1);
    if constexpr (IsInteger_V<T>) {
      switch (type_char) {
      case 'b':
        integer_repr = FormatterIntegerRepresentation::Binary;
        break;
      case 'c':
        integer_repr = FormatterIntegerRepresentation::Character;
        break;
      case 'd':
        integer_repr = FormatterIntegerRepresentation::Decimal;
        break;
      case 'o':
        integer_repr = FormatterIntegerRepresentation::Octal;
        break;
      case 'x':
        integer_repr = FormatterIntegerRepresentation::Hexadecimal;
        break;
      case 'X':
        integer_repr = FormatterIntegerRepresentation::HexadecimalUpper;
        break;
      case 'n':
        integer_repr = FormatterIntegerRepresentation::Number;
        break;
      default:
        break;
      }

      builder.append(value, integer_repr);
    } else if constexpr (IsFloatingPoint_V<T>) {
      switch (type_char) {
      case 'e':
        floating_repr = FormatterFloatingRepresentation::Scientific;
        break;
      case 'E':
        floating_repr = FormatterFloatingRepresentation::ScientificUpper;
        break;
      case 'f':
        floating_repr = FormatterFloatingRepresentation::Fixed;
        break;
      case 'F':
        floating_repr = FormatterFloatingRepresentation::FixedUpper;
        break;
      case 'g':
        floating_repr = FormatterFloatingRepresentation::General;
        break;
      case 'G':
        floating_repr = FormatterFloatingRepresentation::GeneralUpper;
        break;
      case 'n':
        floating_repr = FormatterFloatingRepresentation::Number;
        break;
      case 'p':
        floating_repr = FormatterFloatingRepresentation::Percentage;
        break;
      default:
        break;
      }

      builder.append(value, floating_repr);
    } else {
      builder.append(
          Formatter<RemoveConstReference<decltype(value)>>::format(value));
    }

    fmt = fmt.substring(close_brace + 1);
  }

  template <typename... Args>
    requires(Formattable<Args> && ...)
  String format(String fmt, Args... args) {
    StringBuilder builder{};
    (format_helper(builder, fmt, args), ...);
    builder.append(fmt);
    return builder.build();
  }

  template <typename... Args>
    requires(Formattable<Args> && ...)
  String format(const char *fmt, Args... args) {
    return format(String(fmt), forward<Args>(args)...);
  }

} // namespace Core
