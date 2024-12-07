//
// Created by icxd on 11/11/24.
//

#pragma once

#include <LibCore/Types.hpp>

namespace PIC {

void enable(u8 irq);
void disable(u8 irq);
void eoi(u8 irq);
void init();
u16 get_isr();

} // namespace PIC

class IRQHandlerScope {
public:
  explicit IRQHandlerScope(u8 irq) : m_irq(irq) {}
  ~IRQHandlerScope() { PIC::eoi(m_irq); }

private:
  u8 m_irq;
};
