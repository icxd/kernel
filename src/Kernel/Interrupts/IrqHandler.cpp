//
// Created by icxd on 11/6/24.
//

#include "IrqHandler.hpp"
#include "../PIC.hpp"

IRQHandler::~IRQHandler() {}

void IRQHandler::enable_irq() { PIC::enable(m_irq_number); }

void IRQHandler::disable_irq() { PIC::disable(m_irq_number); }
