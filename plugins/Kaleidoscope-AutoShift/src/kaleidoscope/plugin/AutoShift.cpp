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

// =============================================================================
// AutoShift functions

// -----------------------------------------------------------------------------
// Function to enable AutoShift behavior
void AutoShift::enable() {
  enabled_ = true;
}

// Function to disable AutoShift behavior
void AutoShift::disable() {
  enabled_ = false;
  if (pending_event_.addr.isValid()) {
    Runtime.handlePhysicalKeyEvent(pending_event_);
  }
}

// Toggle the enabled state of the plugin
void AutoShift::toggle() {
  enabled_ = !enabled_;
}

// -----------------------------------------------------------------------------
// Test for whether or not to apply AutoShift to a given `Key`. This function
// will be expanded to include more options for which keys it will apply to.
bool AutoShift::isAutoShiftable(Key key) {
  return key.isKeyboardKey() && !key.isKeyboardModifier();
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

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::AutoShift AutoShift;
