//
// Created by icxd on 11/12/24.
//

#pragma once

enum class FormatterIntegerRepresentation {
  None = 0,
  Binary,
  Character,
  Decimal,
  Octal,
  Hexadecimal,
  HexadecimalUpper,
  Number,
};

enum class FormatterFloatingRepresentation {
  None = 0,
  Scientific,
  ScientificUpper,
  Fixed,
  FixedUpper,
  General,
  GeneralUpper,
  Number,
  Percentage,
};
