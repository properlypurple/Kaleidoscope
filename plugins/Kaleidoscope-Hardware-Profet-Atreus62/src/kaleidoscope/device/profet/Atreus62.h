/* -*- mode: c++ -*-
 * Keyboardio Atreus hardware support for Kaleidoscope
 * Copyright (C) 2019  Keyboard.io, Inc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifdef ARDUINO_AVR_ATREUS62

#include <Arduino.h>

#include "kaleidoscope/driver/bootloader/avr/Caterina.h"
#include "kaleidoscope/device/ATmega32U4Keyboard.h"

namespace kaleidoscope {
namespace device {
namespace keyboardio {


struct AtreusProps : kaleidoscope::device::ATmega32U4KeyboardProps {
  struct KeyScannerProps : public kaleidoscope::driver::keyscanner::ATmegaProps {


    static constexpr uint8_t matrix_rows = 5;
    static constexpr uint8_t matrix_columns = 14;
    typedef MatrixAddr<matrix_rows, matrix_columns> KeyAddr;
#ifndef KALEIDOSCOPE_VIRTUAL_BUILD
    static constexpr uint8_t matrix_row_pins[matrix_rows] =  {PIN_B1, PIN_F7, PIN_F6, PIN_F5, PIN_F4};
    static constexpr uint8_t matrix_col_pins[matrix_columns] = {PIN_B3, PIN_B2, PIN_B6, PIN_B5, PIN_B4, PIN_E6, PIN_D7, PIN_C6, PIN_D4, PIN_D0, PIN_D2, PIN_D3};
#endif // ifndef KALEIDOSCOPE_VIRTUAL_BUILD
  };

  typedef kaleidoscope::driver::keyscanner::ATmega<KeyScannerProps> KeyScanner;
  typedef kaleidoscope::driver::bootloader::avr::Caterina BootLoader;
  static constexpr const char *short_name = "atreus62";
};

#ifndef KALEIDOSCOPE_VIRTUAL_BUILD
class Atreus: public kaleidoscope::device::ATmega32U4Keyboard<AtreusProps> {};
#else // ifndef KALEIDOSCOPE_VIRTUAL_BUILD
/* Device definition omitted for virtual device builds.
 * We need to forward declare the device name, though, as there are
 * some legacy extern references to boards whose definition
 * depends on this.
 */
class Atreus;

#endif // ifndef KALEIDOSCOPE_VIRTUAL_BUILD


#define PER_KEY_DATA(dflt,                                                           \
    R0C0, R0C1, R0C2, R0C3, R0C4, R0C5,             R0C8, R0C9, R0C10, R0C11, R0C12, \
    R1C0, R1C1, R1C2, R1C3, R1C4, R1C5,             R1C8, R1C9, R1C10, R1C11, R1C12, \
    R2C0, R2C1, R2C2, R2C3, R2C4, R2C5,             R2C8, R2C9, R2C10, R2C11, R2C12, \
    R3C0, R3C1, R3C2, R3C3, R3C4, R3C5,             R3C8, R3C9, R3C10, R3C11, R3C12, \
    R4C0, R4C1, R4C2, R4C3, R4C4, R4C5, R4C6, R4C7, R4C8, R4C9, R4C10, R4C11, R4C12  \
  )                                                                                  \
  R0C0, R0C1, R0C2, R0C3, R0C4, R0C5, XXX,  XXX,  R0C8, R0C9, R0C10, R0C11, R0C12, \
  R1C0, R1C1, R1C2, R1C3, R1C4, R1C5, XXX,  XXX,  R1C8, R1C9, R1C10, R1C11, R1C12, \
  R2C0, R2C1, R2C2, R2C3, R2C4, R2C5, XXX,  XXX,  R2C8, R2C9, R2C10, R2C11, R2C12, \
  R3C0, R3C1, R3C2, R3C3, R3C4, R3C5, XXX,  XXX,  R3C8, R3C9, R3C10, R3C11, R3C12, \
  R4C0, R4C1, R4C2, R4C3, R4C4, R4C5, R4C6, R4C7, R4C8, R4C9, R4C10, R4C11, R4C12  \

#define PER_KEY_DATA_STACKED(dflt,                                            \
    R0C0, R0C1, R0C2, R0C3, R0C4, R0C5,                                       \
    R1C0, R1C1, R1C2, R1C3, R1C4, R1C5,                                       \
    R2C0, R2C1, R2C2, R2C3, R2C4, R2C5,                                       \
    R3C0, R3C1, R3C2, R3C3, R3C4, R3C5                                        \
    R4C0, R4C1, R4C2, R3C3, R3C4, R4C5, R4C6,                                 \
                                                                              \
          R0C8, R0C9, R0C10, R0C11, R0C12,                                    \
          R1C8, R1C9, R1C10, R1C11, R1C12,                                    \
          R2C8, R2C9, R2C10, R2C11, R2C12,                                    \
          R3C8, R3C9, R3C10, R3C11, R3C12,                                    \
    R4C7, R4C8, R4C9, R4C10, R4C11, R4C12
  )                                                                           \
  R0C0, R0C1, R0C2, R0C3, R0C4, R0C5, XXX,  XXX,  R0C8, R0C9, R0C10, R0C11, R0C12, \
  R1C0, R1C1, R1C2, R1C3, R1C4, R1C5, XXX,  XXX,  R1C8, R1C9, R1C10, R1C11, R1C12, \
  R2C0, R2C1, R2C2, R2C3, R2C4, R2C5, XXX,  XXX,  R2C8, R2C9, R2C10, R2C11, R2C12, \
  R3C0, R3C1, R3C2, R3C3, R3C4, R3C5, XXX,  XXX,  R3C8, R3C9, R3C10, R3C11, R3C12, \
  R4C0, R4C1, R4C2, R4C3, R4C4, R4C5, R4C6, R4C7, R4C8, R4C9, R4C10, R4C11, R4C12  \
}
}

EXPORT_DEVICE(kaleidoscope::device::Atreus62)

}

#endif
