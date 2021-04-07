/* -*- mode: c++ -*-
 * Kaleidoscope-AutoShift -- Automatic shift on long press
 * Copyright (C) 2021  Keyboard.io, Inc
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
#include "kaleidoscope/KeyEventTracker.h"

namespace kaleidoscope {
namespace plugin {

class AutoShift : public kaleidoscope::Plugin {
 public:
  enum class AutoShiftClass {
    LETTERS = 0b0001,
    NUMBERS = 0b0010,
    SPECIAL = 0b0100
  };

  static void enable() {
    enabled_ = true;
  }
  static void disable();
  static void toggle() {
    if (enabled_)
      disable();
    else
      enable();
  }

  static void setTimeout(uint16_t new_timeout) {
    timeout_ = new_timeout;
  }
  static uint16_t timeout() {
    return timeout_;
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
  static void setEnabledClasses(uint8_t classes) {
    enabled_classes_ = classes;
  }
  static uint8_t enabledClasses() {
    return enabled_classes_;
  }

  static bool isAutoShiftable(Key key);
  static bool enabledForKey(Key key);

  EventHandlerResult onPhysicalKeyEvent(KeyEvent &event);
  EventHandlerResult afterEachCycle();

 private:
  static bool enabled_;
  static uint16_t timeout_;
  static uint8_t enabled_classes_;
  static uint16_t start_time_;

  static KeyEventTracker event_tracker_;
};

class AutoShiftConfiguration: public kaleidoscope::Plugin {
 public:
  AutoShiftConfiguration() {}

  EventHandlerResult onSetup();
  EventHandlerResult onFocusEvent(const char *command);

 private:
  typedef struct {
    bool disabled;
    uint16_t timeout;
    uint8_t enabled_classes;
  } settings_t;
  static settings_t settings_;
  uint16_t settings_base_;
};

} // namespace plugin
} // namespace kaleidoscope

extern kaleidoscope::plugin::AutoShift AutoShift;
extern kaleidoscope::plugin::AutoShiftConfiguration AutoShiftConfiguration;
