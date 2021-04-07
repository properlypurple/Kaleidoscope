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
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-FocusSerial.h>

#include "kaleidoscope/Runtime.h"
#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/key_events.h"

namespace kaleidoscope {
namespace plugin {

/********* AutoShift *********/

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

/********* AutoShiftConfiguration *********/
AutoShiftConfiguration::settings_t AutoShiftConfiguration::settings_ = {
  .disabled = false,
  .delay = 500,
  .enabled_classes = (uint8_t)AutoShift::AutoShiftClass::LETTERS |
    (uint8_t)AutoShift::AutoShiftClass::NUMBERS |
    (uint8_t)AutoShift::AutoShiftClass::SPECIAL
};

EventHandlerResult AutoShiftConfiguration::onSetup() {
  settings_base_ = ::EEPROMSettings.requestSlice(sizeof(settings_));
  uint32_t checker;

  Runtime.storage().get(settings_base_, checker);

  // Check if we have an empty eeprom...
  if (checker == 0xffffffff) {
    // ...if the eeprom was empty, store the default settings.
    Runtime.storage().put(settings_base_, settings_);
    Runtime.storage().commit();
  }

  Runtime.storage().get(settings_base_, settings_);
  return EventHandlerResult::OK;
}

EventHandlerResult AutoShiftConfiguration::onFocusEvent(const char *command) {
  enum {
    DISABLED,
    DELAY,
    CLASSES
  } subCommand;

  if (::Focus.handleHelp(command, PSTR("autoshift.disabled\n"
                                       "autoshift.delay\n"
                                       "autoshift.classes")))
    return EventHandlerResult::OK;

  if (strncmp_P(command, PSTR("autoshift."), 10) != 0)
    return EventHandlerResult::OK;
  if (strcmp_P(command + 10, PSTR("disabled")) == 0)
    subCommand = DISABLED;
  else if (strcmp_P(command + 10, PSTR("delay")) == 0)
    subCommand = DELAY;
  else if (strcmp_P(command + 10, PSTR("classes")) == 0)
    subCommand = CLASSES;
  else
    return EventHandlerResult::OK;

  switch (subCommand) {
    case DISABLED:
      if (::Focus.isEOL()) {
        ::Focus.send(settings_.disabled);
      } else {
        uint8_t v;
        ::Focus.read(v);

        if (v) {
          settings_.disabled = true;
          ::AutoShift.disable();
        } else {
          settings_.disabled = false;
          ::AutoShift.enable();
        }
      }
      break;

    case DELAY:
      if (::Focus.isEOL()) {
        ::Focus.send(settings_.delay);
      } else {
        ::Focus.read(settings_.delay);
        ::AutoShift.setDelay(settings_.delay);
      }
      break;

    case CLASSES:
      if (::Focus.isEOL()) {
        ::Focus.send(settings_.enabled_classes);
      } else {
        ::Focus.read(settings_.enabled_classes);
        ::AutoShift.setEnabledClasses(settings_.enabled_classes);
      }
      break;
  }

  Runtime.storage().put(settings_base_, settings_);
  Runtime.storage().commit();
  return EventHandlerResult::EVENT_CONSUMED;
}

}
}

kaleidoscope::plugin::AutoShift AutoShift;
kaleidoscope::plugin::AutoShiftConfiguration AutoShiftConfiguration;
