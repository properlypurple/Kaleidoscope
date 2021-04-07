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

#include <Kaleidoscope-AutoShift.h>
#include <Kaleidoscope-EEPROM-Settings.h>
#include <Kaleidoscope-FocusSerial.h>

#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/KeyAddr.h"
#include "kaleidoscope/key_defs.h"
#include "kaleidoscope/KeyEvent.h"
#include "kaleidoscope/LiveKeys.h"
#include "kaleidoscope/Runtime.h"

namespace kaleidoscope {
namespace plugin {

// =============================================================================
// AutoShift static class variables

// If AutoShift is turned on, this will be set to `true`.
bool AutoShift::enabled_ = true;

// The length of time an auto-shiftable key will delay before producing an event
// with its shift state reversed.
uint16_t AutoShift::timeout_ = 175;

// `start_time_` is a timestamp of when the pending keypress was detected. It's
// used to determine if the press has timed out.
uint16_t AutoShift::start_time_ = 0;

// The event tracker ensures that the `onPhysicalKeyEvent()` handler will follow
// the rules in order to avoid interference with other plugins and prevent
// processing the same event more than once.
KeyEventTracker AutoShift::event_tracker_;

// If there's a delayed keypress from AutoShift, this stored event will contain
// a valid `KeyAddr`. The default constructor produces an event addr of
// `KeyAddr::none()`, so the plugin will start in an inactive state.
KeyEvent pending_event_;

// By default, let AutoShift handle all supported classes of keys.
uint8_t AutoShift::enabled_classes_ =
  (uint8_t)AutoShift::AutoShiftClass::LETTERS |
  (uint8_t)AutoShift::AutoShiftClass::NUMBERS |
  (uint8_t)AutoShift::AutoShiftClass::SPECIAL;

// =============================================================================
// AutoShift functions

void AutoShift::disable() {
  enabled_ = false;
  if (pending_event_.addr.isValid()) {
    Runtime.handlePhysicalKeyEvent(pending_event_);
  }
}

// -----------------------------------------------------------------------------
// Test for whether or not to apply AutoShift to a given `Key`. This function
// can be overridden from the user sketch.
__attribute__((weak)) bool AutoShift::isAutoShiftable(Key key) {
  return enabledForKey(key);
}

// The default method that determines whether a particular key is an auto-shift
// candidate. Used by `isAutoShiftable()`, separate to allow re-use when the
// caller is overridden.
bool AutoShift::enabledForKey(Key key) {
  // We only support auto-shifting keyboard keys.
  if (!key.isKeyboardKey())
    return false;

  // We do not support auto-shifting modifiers alone.
  if (key.isKeyboardModifier())
    return false;

  // We will be comparing the current key _without_ flags, for the sake of
  // simplicity.
  Key k = key;
  k.setFlags(0);

  if (isClassEnabled(AutoShiftClass::LETTERS)) {
    if (k >= Key_A && k <= Key_Z) return true;
  }
  if (isClassEnabled(AutoShiftClass::NUMBERS)) {
    if (k >= Key_1 && k <= Key_0) return true;
  }
  if (isClassEnabled(AutoShiftClass::SPECIAL)) {
    if (k >= Key_Minus && k <= Key_Slash) return true;
    if (k == Key_NonUsBackslashAndPipe) return true;
  }

  return false;
}

// =============================================================================
// Event handler hook functions

// -----------------------------------------------------------------------------
EventHandlerResult AutoShift::onPhysicalKeyEvent(KeyEvent &event) {
  // If AutoShift has already processed and released this event, ignore
  // it. There's no need to update the event tracker in this one case.
  if (event_tracker_.shouldIgnore(event))
    return EventHandlerResult::OK;

  // Do nothing if disabled.
  if (!enabled_)
    return event_tracker_.done(event, EventHandlerResult::OK);

  // If event.addr is not a physical key, ignore it; some other plugin injected
  // it. This check should be unnecessary.
  if (!event.addr.isValid() || (event.state & INJECTED) != 0) {
    return EventHandlerResult::OK;
  }

  if (keyToggledOn(event.state)) {
    // First, if there's a pending event, send it without adding `shift`, and
    // clear that event.
    if (pending_event_.addr.isValid()) {
      Runtime.handlePhysicalKeyEvent(pending_event_);
      pending_event_.addr.clear();
    }

    if (isAutoShiftable(event.key)) {
      pending_event_ = event;
      start_time_ = Runtime.millisAtCycleStart();
      return event_tracker_.done(event, EventHandlerResult::ABORT);
    }
  } else {
    // A key was released
    if (pending_event_.addr.isValid()) {
      if (event.addr == pending_event_.addr) {
        // The pending AutoShift key has been released before timing out, so we
        // send its press event unaltered.
        Runtime.handlePhysicalKeyEvent(pending_event_);
        pending_event_.addr.clear();
      } else {
        // Some other key was released while an AutoShift press was still
        // pending. We release the event out of order, which should probably be
        // considered a bug. This can be fixed by switching to a
        // KeyAddrEventQueue.
        event.swapId(pending_event_);
      }
    }
  }
  return event_tracker_.done(event, EventHandlerResult::OK);
}

// -----------------------------------------------------------------------------
EventHandlerResult AutoShift::afterEachCycle() {
  // If there's a pending AutoShift event, and it has timed out, we need to
  // release the event with the `shift` flag applied.
  if (pending_event_.addr.isValid() &&
      Runtime.hasTimeExpired(start_time_, timeout_)) {
    // Toggle the state of the `SHIFT_HELD` bit in the modifier flags for the
    // key for the pending event.
    uint8_t flags = pending_event_.key.getFlags();
    flags ^= SHIFT_HELD;
    pending_event_.key.setFlags(flags);
    // Send the event with the altered shift state.
    Runtime.handlePhysicalKeyEvent(pending_event_);
    // Clear the pending event.
    pending_event_.addr.clear();
  }
  return EventHandlerResult::OK;
}

/********* AutoShiftConfiguration *********/

AutoShiftConfiguration::settings_t AutoShiftConfiguration::settings_ = {
  .disabled = false,
  .timeout = 175,
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
    TIMEOUT,
    CLASSES
  } subCommand;

  if (::Focus.handleHelp(command, PSTR("autoshift.disabled\n"
                                       "autoshift.timeout\n"
                                       "autoshift.classes")))
    return EventHandlerResult::OK;

  if (strncmp_P(command, PSTR("autoshift."), 10) != 0)
    return EventHandlerResult::OK;
  if (strcmp_P(command + 10, PSTR("disabled")) == 0)
    subCommand = DISABLED;
  else if (strcmp_P(command + 10, PSTR("timeout")) == 0)
    subCommand = TIMEOUT;
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

  case TIMEOUT:
    if (::Focus.isEOL()) {
      ::Focus.send(settings_.timeout);
    } else {
      ::Focus.read(settings_.timeout);
      ::AutoShift.setTimeout(settings_.timeout);
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

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::AutoShift AutoShift;
kaleidoscope::plugin::AutoShiftConfiguration AutoShiftConfiguration;
