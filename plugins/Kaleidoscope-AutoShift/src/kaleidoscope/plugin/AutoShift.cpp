/* -*- mode: c++ -*-
 * Kaleidoscope-AutoShift -- AutoShift support for Kaleidoscope
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

#include <Kaleidoscope-AutoShift.h>
#include "kaleidoscope/Runtime.h"
#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/key_events.h"

namespace kaleidoscope {
namespace plugin {

bool AutoShift::disabled_;
bool AutoShift::drop_shift_;
uint16_t AutoShift::delay_ = 500;
uint32_t AutoShift::start_time_;
uint8_t AutoShift::enabled_classes_ =
    (uint8_t)AutoShift::AutoShiftClass::LETTERS |
    (uint8_t)AutoShift::AutoShiftClass::NUMBERS |
    (uint8_t)AutoShift::AutoShiftClass::SPECIAL;

EventHandlerResult AutoShift::onKeyswitchEvent(Key &key, KeyAddr key_addr, uint8_t key_state) {
  // If the plugin is disabled, or isn't enabled for the current key, bail out.
  if (disabled_ || !enabledForKey(key)) {
    return EventHandlerResult::OK;
  }

  // If we have a possible AutoShift going on, and have more than one key
  // pressed, abort the autoshift.
  if (Runtime.device().pressedKeyswitchCount() > 1) {
    if (start_time_) {
      drop_shift_ = true;
    }
    start_time_ = 0;
    return EventHandlerResult::OK;
  }

  // We have a single key pressed, and AutoShift is enabled for it.

  // If we toggled on, record the start time and consume the event.
  if (keyToggledOn(key_state)) {
    start_time_ = Runtime.millisAtCycleStart();
  }

  // If we're being held past the elapsed time, auto-shift.
  if (Runtime.hasTimeExpired(start_time_, delay_)) {
    Runtime.hid().keyboard().pressKey(Key_LeftShift);

    // If we're toggling off, reset the timer, and drop shift after the report
    // is sent.
    if (keyToggledOff(key_state)) {
      drop_shift_ = true;
      start_time_ = 0;
    }

    // ...otherwise, with shift pressed, just proceed.
    return EventHandlerResult::OK;
  }

  // If we're _not_ held past the elapsed time, and we toggle off, send the
  // press event, reset the timer, and proceed - this way the host will get both
  // the press and the release event in proper order.
  if (keyToggledOff(key_state)) {
    Runtime.hid().keyboard().pressKey(key);
    Runtime.hid().keyboard().sendReport();
    ::delay(10);

    start_time_ = 0;
    return EventHandlerResult::OK;
  }

  // In any other case, consume the event.
  return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult AutoShift::afterEachCycle() {
  if (drop_shift_) {
    Runtime.hid().keyboard().releaseKey(Key_LeftShift);
    Runtime.hid().keyboard().sendReport();
    ::delay(10);
    drop_shift_ = false;
  }
  return EventHandlerResult::OK;
}

__attribute__((weak)) bool AutoShift::enabledForKey(Key key) {
  // AutoShift is only enabled for flag-less keys.
  if (key.getFlags())
    return false;

  if (isClassEnabled(AutoShiftClass::LETTERS)) {
    if (key >= Key_A && key <= Key_Z) return true;
  }
  if (isClassEnabled(AutoShiftClass::NUMBERS)) {
    if (key >= Key_1 && key <= Key_0) return true;
  }
  if (isClassEnabled(AutoShiftClass::SPECIAL)) {
    if (key >= Key_Minus && key <= Key_Slash) return true;
    if (key == Key_NonUsBackslashAndPipe) return true;
  }

  return false;
}

}
}

kaleidoscope::plugin::AutoShift AutoShift;
