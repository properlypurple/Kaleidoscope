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

#include <Kaleidoscope-AutoShift.h>
#include "kaleidoscope/Runtime.h"
#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/key_events.h"

namespace kaleidoscope {
namespace plugin {

bool AutoShift::disabled_;
bool AutoShift::drop_shift_;
KeyAddr AutoShift::autoshift_addr_ = UnknownKeyswitchLocation;
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

  // When a key toggles on, record the start time, and send the key. If we end
  // up auto shifting, we'll backspace it.
  if (keyToggledOn(key_state)) {
    start_time_ = Runtime.millisAtCycleStart();
    autoshift_addr_ = key_addr;
    return EventHandlerResult::OK;
  }

  // Releasing a key counts as accepting its current state, which we signal with
  // setting the start time to zero.
  if (keyToggledOff(key_state)) {
    start_time_ = 0;
  }

  // If a key is being held, check if it is time to shift it.
  if (keyWasPressed(key_state) && start_time_ != 0 && key_addr == autoshift_addr_) {
    if (Runtime.hasTimeExpired(start_time_, delay_)) {
      // The delay expired, time to backspace back and send it shifted. We delay
      // between press & release, so the host can properly register it.
      Runtime.hid().keyboard().pressRawKey(Key_Backspace);
      Runtime.hid().keyboard().sendReport();
      ::delay(10);
      Runtime.hid().keyboard().releaseRawKey(Key_Backspace);
      // FIXME(algernon): We send a shift separately, for reasons explained
      // below.
      Runtime.hid().keyboard().pressRawKey(Key_LeftShift);
      Runtime.hid().keyboard().sendReport();
      ::delay(10);

      // FIXME(algernon): For some odd reason, this doesn't seem to send
      // Shift+Key, and we have to do the shifting dance separately.
      key = LSHIFT(key);
      start_time_ = 0;
      drop_shift_ = true;

      return EventHandlerResult::OK;
    }
  }

  // If we haven't timed out yet, or cancelled the auto-shift by release,
  // consume the event to avoid the host-side auto-repeat.
  return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult AutoShift::afterEachCycle() {
  if (drop_shift_) {
    Runtime.hid().keyboard().releaseRawKey(Key_LeftShift);
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
