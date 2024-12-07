//
// Created by icxd on 11/6/24.
//


#pragma once

#include <LibCore/Types.hpp>

class IRQHandler {
public:
  virtual ~IRQHandler();
  virtual void handle_irq() = 0;

  [[nodiscard]] u8 irq_number() const { return m_irq_number; }

  void enable_irq();
  void disable_irq();

protected:
  explicit IRQHandler(u8 irq_number) : m_irq_number(irq_number) {}

private:
  u8 m_irq_number { 0 };
};
