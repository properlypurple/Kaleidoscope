/* -*- mode: c++ -*-
 * Kaleidoscope-AutoShift -- AutoShift support for Kaleidoscope
 * Copyright (C) 2019  Jared Lindsay
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

#include "kaleidoscope/Runtime.h"

namespace kaleidoscope {
namespace plugin {

class AutoShift: public kaleidoscope::Plugin {
 public:
  AutoShift() {}

  enum class AutoShiftClass {
    LETTERS = 0b0001,
    NUMBERS = 0b0010,
    SPECIAL = 0b0100
  };

  static void enable() {
    disabled_ = false;
  }
  static void disable() {
    disabled_ = true;
  }

  static void enableClass(AutoShiftClass cls) {
    enabled_classes_ |= (uint8_t)cls;
  }
  static void disableClass(AutoShiftClass cls) {
    enabled_classes_ &= ~((uint8_t)cls);
  }
  static bool isClassEnabled(AutoShiftClass cls) {
    return enabled_classes_ & (uint8_t)cls;
  }
  static uint8_t enabledClasses() {
    return enabled_classes_;
  }

  static void setDelay(uint16_t new_delay) {
    delay_ = new_delay;
  }
  static uint16_t delay() {
    return delay_;
  }

  EventHandlerResult onKeyswitchEvent(Key &key, KeyAddr key_addr, uint8_t key_state);
  static bool enabledForKey(Key key);
  EventHandlerResult afterEachCycle();

 private:
  static bool disabled_;
  static uint8_t enabled_classes_;
  static uint16_t delay_;
  static uint32_t start_time_;
  static bool drop_shift_;
  static KeyAddr autoshift_addr_;
};

}
}

extern kaleidoscope::plugin::AutoShift AutoShift;
