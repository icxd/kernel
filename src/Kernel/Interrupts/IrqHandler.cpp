//
// Created by icxd on 11/6/24.
//

#include "IrqHandler.hpp"

IRQHandler::~IRQHandler() {
}

void IRQHandler::enable_irq() {
  // TODO: PIC::enable(m_irq_number);
}

void IRQHandler::disable_irq() {
  // TODO: PIC::disable(m_irq_number);
}
