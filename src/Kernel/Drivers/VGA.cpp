//
// Created by icxd on 10/29/24.
//

#include "VGA.hpp"

void VGA::clear() {
  for (usz i = 0; i < VGA_SCREEN_WIDTH * VGA_SCREEN_HEIGHT - 1; i += 2) {
    m_ptr[i] = ' ';
    m_ptr[i + 1] = m_color;
  }
}

void VGA::putchar(char character) {
  if (character == '\n')
    return put_newline();

  m_ptr[m_x + m_y * VGA_SCREEN_WIDTH] = character;
  m_ptr[m_x + m_y * VGA_SCREEN_WIDTH + 1] = m_color;

  m_x += 2;
  if (m_x == VGA_SCREEN_WIDTH) {
    m_x = 0;
    m_y++;
    if (m_y == VGA_SCREEN_HEIGHT) {
      m_y = 0;
    }
  }
}

void VGA::puts(const char *string) {
  while (*string) putchar(*string++);
}

void VGA::put_newline() {
  m_x = 0;
  m_y++;
  if (m_y == VGA_SCREEN_HEIGHT) {
    m_y = 0;
  }
}
