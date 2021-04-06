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
#include "kaleidoscope/KeyEvent.h"

namespace kaleidoscope {

typedef uint8_t KeyEventId;

class KeyEventTracker {

 private:
  KeyEventId last_id_done_ = -1;

 public:
  bool shouldIgnore(const KeyEvent &event) const {
    int8_t offset = event.id - last_id_done_;
    return offset <= 0;
  }

  EventHandlerResult done(KeyEvent &event, EventHandlerResult result) {
    if (event.id <= last_id_done_)
      event.id = last_id_done_ + 1;
    last_id_done_ = event.id;
    return result;
  }

};

}  // namespace kaleidoscope

typedef kaleidoscope::KeyEventId KeyEventId;
typedef kaleidoscope::KeyEvent KeyEvent;
