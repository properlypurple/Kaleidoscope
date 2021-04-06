/* Kaleidoscope - Firmware for computer input devices
 * Copyright (C) 2021  Keyboard.io, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <Arduino.h>
#include "kaleidoscope/key_defs.h"
#include "kaleidoscope/KeyAddr.h"

namespace kaleidoscope {

typedef uint8_t KeyEventId;

struct KeyEvent {

 private:
  // serial number of the event:
  static KeyEventId next_id_;

 public:
  KeyEvent() = default;
  // constructor for keyscanner use:
  KeyEvent(KeyAddr addr, uint8_t state)
    : addr(addr), state(state), key(Key_NoKey) {
    id = next_id_++;
  }
  // constructor for plugin use when generating a new event:
  KeyEvent(KeyAddr addr, uint8_t state, Key key)
    : addr(addr), state(state), key(key) {
    id = next_id_++;
  }
  // constructor for plugin use when regenerating an event:
  KeyEvent(KeyAddr addr, uint8_t state, Key key, KeyEventId id)
    : addr(addr), state(state), key(key), id(id) {}

  KeyAddr addr = KeyAddr::none();
  uint8_t state = 0;
  Key key = Key_NoKey;
  KeyEventId id = -1;

  void swapId(KeyEvent &other) {
    KeyEventId tmp_id = id;
    id = other.id;
    other.id = tmp_id;
  }
};

}  // namespace kaleidoscope

typedef kaleidoscope::KeyEventId KeyEventId;
typedef kaleidoscope::KeyEvent KeyEvent;
