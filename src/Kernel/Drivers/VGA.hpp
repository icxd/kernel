//
// Created by icxd on 10/29/24.
//

#pragma once

#include <LibCore/Types.hpp>
#include <LibCore/String.hpp>
#include <LibCpp/type_traits.hpp>

enum class VGAColor : u8 {
  Black,
  Blue,
  Green,
  Cyan,
  Red,
  Purple,
  Brown,
  Gray,
  DarkGray,
  LightBlue,
  LightGreen,
  LightCyan,
  LightRed,
  LightPurple,
  Yellow,
  White,
};

#define BG(x) (std::to_underlying(x) << 4)
#define FG(x) (std::to_underlying(x) & 0xf)
#define COLOR(bg, fg) (BG(bg) | FG(fg))

static constexpr u8 VGA_SCREEN_WIDTH = 80 * 2;
static constexpr u8 VGA_SCREEN_HEIGHT = 25;

class VGA {
public:
  VGA()
      : m_ptr((u8 *) 0xb8000)
      , m_x(0), m_y(0)
      , m_color(COLOR(VGAColor::White, VGAColor::Black))
  {}

  void clear();

  void putchar(char character);
  void puts(const char *string);
  inline void puts(Core::String &s) { puts(s.characters()); }

  [[nodiscard]] u8 *ptr() const { return m_ptr; }
  [[nodiscard]] usz x() const { return m_x; }
  [[nodiscard]] usz y() const { return m_y; }
  [[nodiscard]] u8 color() const { return m_color; }
  void set_color(u8 color) { m_color = color; }

private:
  void put_newline();

private:
  u8 *m_ptr;
  usz m_x, m_y;
  u8 m_color;
};
