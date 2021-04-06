/* -*- mode: c++ -*-
 * Kaleidoscope-SpaceCadet -- Space Cadet Shift Extended
 * Copyright (C) 2016, 2017, 2018  Keyboard.io, Inc, Ben Gemperline
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

#include <Kaleidoscope-SpaceCadet.h>
#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/key_events.h"

namespace kaleidoscope {
namespace plugin {

// Constructor with input and output, and assume default timeout
SpaceCadet::KeyBinding::KeyBinding(Key input, Key output)
  : input(input), output(output) {}

// Constructor with all three set
SpaceCadet::KeyBinding::KeyBinding(Key input, Key output, uint16_t timeout)
  : input(input), output(output), timeout(timeout) {}

// =============================================================================
// Space Cadet class variables
SpaceCadet::KeyBinding * SpaceCadet::map;
uint16_t SpaceCadet::time_out = 1000;
bool SpaceCadet::disabled = false;

// These variables are used to keep track of any pending unresolved SpaceCadet
// key that has been pressed. If `pending_map_index_` is negative, it means
// there is no such pending keypress. Otherwise, it holds the value of the index
// of that key in the array.
int8_t SpaceCadet::pending_map_index_ = -1;
// `pending_event_` holds a copy of the pending event.
KeyEvent SpaceCadet::pending_event_;
// `start_time_` is a timestamp of when the pending keypress was detected. It's
// used to determine if the press has timed out.
uint16_t SpaceCadet::start_time_ = 0;

KeyEventTracker SpaceCadet::event_tracker_;

// =============================================================================
// SpaceCadet functions

// Constructor
SpaceCadet::SpaceCadet() {
  static SpaceCadet::KeyBinding initialmap[] = {
    // By default, respect the default timeout
    {Key_LeftShift, Key_LeftParen, 0},
    {Key_RightShift, Key_RightParen, 0},
    // These may be uncommented, added, or set in the main sketch
    /*
    {Key_LeftGui, Key_LeftCurlyBracket, 250},
    {Key_RightAlt, Key_RightCurlyBracket, 250},
    {Key_LeftControl, Key_LeftBracket, 250},
    {Key_RightControl, Key_RightBracket, 250},
    */
    SPACECADET_MAP_END
  };

  map = initialmap;
}

// -----------------------------------------------------------------------------
// Function to determine whether SpaceCadet is active (useful for Macros and
// other plugins).
bool SpaceCadet::active() {
  return !disabled;
}

// Function to enable SpaceCadet behavior
void SpaceCadet::enable() {
  disabled = false;
}

// Function to disable SpaceCadet behavior
void SpaceCadet::disable() {
  disabled = true;
}

// =============================================================================
// Event handler hook functions

// -----------------------------------------------------------------------------
EventHandlerResult SpaceCadet::onPhysicalKeyEvent(KeyEvent &event) {
  // If SpaceCadet has already processed and released this event, ignore
  // it. There's no need to update the event tracker in this one case.
  if (event_tracker_.shouldIgnore(event))
    return EventHandlerResult::OK;

  // Turn SpaceCadet on or off.
  if (keyToggledOn(event.state)) {
    if (event.key == Key_SpaceCadetEnable) {
      enable();
      return event_tracker_.done(event, EventHandlerResult::EVENT_CONSUMED);
    }
    if (event.key == Key_SpaceCadetDisable) {
      disable();
      return event_tracker_.done(event, EventHandlerResult::EVENT_CONSUMED);
    }
  }

  // Do nothing if disabled, but keep the event tracker current.
  if (disabled)
    return event_tracker_.done(event, EventHandlerResult::OK);

  // If event.addr is not a physical key, ignore it; some other plugin injected
  // it. This check should be unnecessary.
  if (!event.addr.isValid() || (event.state & INJECTED) != 0) {
    return EventHandlerResult::OK;
  }

  if (keyToggledOn(event.state)) {
    if (pending_map_index_ >= 0) {
      // There was already a pending SpaceCadet key; interrupt it by releasing
      // its event unchanged now.
      Runtime.handlePhysicalKeyEvent(pending_event_);
    }
    // Check to see if the newly-pressed key is a SpaceCadet key.
    pending_map_index_ = getSpaceCadetKeyIndex(event.key);
    if (pending_map_index_ >= 0) {
      pending_event_ = event;
      start_time_ = Runtime.millisAtCycleStart();
      return event_tracker_.done(event, EventHandlerResult::ABORT);
    }
  } else {
    if (pending_map_index_ >= 0 && event.addr == pending_event_.addr) {
      // The pending key got released within its timeout period. Release it with
      // the alternate key value, then send its release event:
      pending_event_.key = map[pending_map_index_].output;
      Runtime.handlePhysicalKeyEvent(pending_event_);
      // Finally, clear state:
      pending_map_index_ = -1;
    }
  }
  // Here, we might be letting a release event through while the SpaceCadet key
  // is still held, but before it times out. If the released key is a modifier,
  // this could be considered to be a rollover bug, where the modifier should
  // apply to the SpaceCadet key, but doesn't. Furthermore, if this is the case,
  // we're changing the order of events, and we're required to release strictly
  // increasing event ids, so we need to replace the id of the event.
  if (pending_map_index_ >= 0 &&
      keyToggledOff(event.state) &&
      event.addr != pending_event_.addr) {
    // Swap the event's id with the id of the pending event in order to
    // guarantee that event IDs are monotonically increasing.
    event.swapId(pending_event_);
  }
  return event_tracker_.done(event, EventHandlerResult::OK);
}

// -----------------------------------------------------------------------------
EventHandlerResult SpaceCadet::afterEachCycle() {
  // If there's no pending event, return.
  if (pending_map_index_ < 0)
    return EventHandlerResult::OK;
  // Get timeout value for the pending key.
  uint16_t pending_timeout = time_out;
  if (map[pending_map_index_].timeout != 0)
    pending_timeout = map[pending_map_index_].timeout;
  if (Runtime.hasTimeExpired(start_time_, pending_timeout)) {
    // The timer has expired; release the pending event unchanged.
    event_tracker_.done(pending_event_, EventHandlerResult::OK);
    Runtime.handlePhysicalKeyEvent(pending_event_);
    pending_map_index_ = -1;
  }
  return EventHandlerResult::OK;
}

// =============================================================================
// Private helper function(s)

int8_t SpaceCadet::getSpaceCadetKeyIndex(Key key) const {
  for (uint8_t i = 0; !map[i].isEmpty(); ++i) {
    if (map[i].input == key) {
      return i;
    }
  }
  return -1;
}

} // namespace plugin
} // namespace kaleidoscope

kaleidoscope::plugin::SpaceCadet SpaceCadet;
