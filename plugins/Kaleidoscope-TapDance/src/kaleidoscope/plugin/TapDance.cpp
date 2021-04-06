/* -*- mode: c++ -*-
 * Kaleidoscope-TapDance -- Tap-dance keys
 * Copyright (C) 2016, 2017, 2018  Keyboard.io, Inc
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

#include <Kaleidoscope-TapDance.h>
#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/layers.h"

namespace kaleidoscope {
namespace plugin {

// --- config ---

uint16_t TapDance::time_out = 200;
uint8_t TapDance::tap_count_ = 0;

// --- api ---
void TapDance::actionKeys(uint8_t tap_count,
                          ActionType action,
                          uint8_t max_keys,
                          const Key tap_keys[]) {
  if (event_queue_.isEmpty())
    return;

  if (tap_count > max_keys)
    tap_count = max_keys;

  KeyEvent event = event_queue_.event(0);
  event.key = tap_keys[tap_count - 1].readFromProgmem();

  if (action == Interrupt || action == Timeout) {
    Runtime.handlePhysicalKeyEvent(event);
    event_queue_.shift();
  } else if (action == Tap && tap_count == max_keys) {
    Runtime.handlePhysicalKeyEvent(event);
    event_queue_.clear();
    tap_count_ = 0;
  }
}


void TapDance::flushQueue(KeyAddr ignored_addr) {
  while (! event_queue_.isEmpty()) {
    KeyEvent queued_event = event_queue_.event(0);
    if (queued_event.addr != ignored_addr)
      Runtime.handlePhysicalKeyEvent(queued_event);
    event_queue_.shift();
  }
}

// --- hooks ---

EventHandlerResult TapDance::onPhysicalKeyEvent(KeyEvent &event) {
  // If event.addr is not a physical key, ignore it; some other plugin injected it.
  if (! event.addr.isValid() || (event.state & INJECTED) != 0) {
    return EventHandlerResult::OK;
  }

  // If TapDance has already processed and released this event, ignore it.
  if (! event_queue_.isEmpty()) {
    int8_t event_id_offset = event.id - event_queue_.id(event_queue_.length() - 1);
    if (event_id_offset <= 0) {
      return EventHandlerResult::OK;
    }
  }

  if (keyToggledOff(event.state)) {
    if (event_queue_.isEmpty())
      return EventHandlerResult::OK;
    event_queue_.append(event);
    return EventHandlerResult::ABORT;
  }

  if (event_queue_.isEmpty() && !isTapDanceKey(event.key))
    return EventHandlerResult::OK;

  KeyAddr td_addr = event_queue_.addr(0);
  Key     td_key = Layer.lookupOnActiveLayer(td_addr);
  uint8_t td_id = td_key.getRaw() - ranges::TD_FIRST;

  if (! event_queue_.isEmpty() &&
      event.addr != event_queue_.addr(0)) {
    // Interrupt: Call `tapDanceAction()` first, so it will have access to the
    // TapDance key press event that needs to be sent, then flush the queue.
    tapDanceAction(td_id, td_addr, tap_count_, Interrupt);
    flushQueue();
    tap_count_ = 0;
    // If the event isn't another TapDance key, let it proceed. If it is, fall
    // through to the next block, which handles "Tap" actions.
    if (! isTapDanceKey(event.key))
      return EventHandlerResult::OK;
  }

  // Tap: First flush the queue, ignoring the previous press and release events
  // for the TapDance key, then add the new tap to the queue (it becomes the
  // first entry).
  flushQueue(event.addr);
  event_queue_.append(event);
  tapDanceAction(td_id, td_addr, ++tap_count_, Tap);
  return EventHandlerResult::ABORT;
}

EventHandlerResult TapDance::afterEachCycle() {
  // If there's no active TapDance sequence, there's nothing to do.
  if (event_queue_.isEmpty())
    return EventHandlerResult::OK;

  // The first event in the queue is now guaranteed to be a TapDance key.
  KeyAddr td_addr = event_queue_.addr(0);
  Key     td_key = Layer.lookupOnActiveLayer(td_addr);
  uint8_t td_id = td_key.getRaw() - ranges::TD_FIRST;

  // Check for timeout
  uint16_t start_time = event_queue_.timestamp(0);
  if (Runtime.hasTimeExpired(start_time, time_out)) {
    tapDanceAction(td_id, td_addr, tap_count_, Timeout);
    flushQueue();
    tap_count_ = 0;
  }
  return EventHandlerResult::OK;
}

}  // namespace plugin
}  // namespace kaleidoscope

__attribute__((weak)) void tapDanceAction(uint8_t tap_dance_index, KeyAddr key_addr, uint8_t tap_count,
                                          kaleidoscope::plugin::TapDance::ActionType tap_dance_action) {
}

kaleidoscope::plugin::TapDance TapDance;
