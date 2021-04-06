/* -*- mode: c++ -*-
 * Kaleidoscope-Syster -- Symbolic input system
 * Copyright (C) 2017, 2018, 2019  Keyboard.io, Inc
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

#include <Kaleidoscope-Syster.h>
#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/key_events.h"

#undef SYSTER

namespace kaleidoscope {
namespace plugin {

// --- state ---
char Syster::symbol_[SYSTER_MAX_SYMBOL_LENGTH + 1];
uint8_t Syster::symbol_pos_;
bool Syster::is_active_;

// --- helpers ---
#define isSyster(k) (k == kaleidoscope::ranges::SYSTER)

// --- api ---
void Syster::reset(void) {
  symbol_pos_ = 0;
  symbol_[0] = 0;
  is_active_ = false;
}

bool Syster::is_active(void) {
  return is_active_;
}

// --- hooks ---
EventHandlerResult Syster::onKeyEvent(KeyEvent &event) {
  if (!is_active_) {
    if (!isSyster(event.key))
      return EventHandlerResult::OK;

    if (keyToggledOn(event.state)) {
      is_active_ = true;
      systerAction(StartAction, nullptr);
    }
    return EventHandlerResult::EVENT_CONSUMED;
  }

  if (event.state & INJECTED)
    return EventHandlerResult::OK;

  if (isSyster(event.key)) {
    return EventHandlerResult::EVENT_CONSUMED;
  }

  if (event.key == Key_Backspace && symbol_pos_ == 0) {
    return EventHandlerResult::EVENT_CONSUMED;
  }

  if (keyToggledOff(event.state)) {
  }

  if (keyToggledOn(event.state)) {
    if (event.key == Key_Spacebar) {
      eraseChars(symbol_pos_);

      systerAction(EndAction, nullptr);

      symbol_[symbol_pos_] = 0;
      systerAction(SymbolAction, symbol_);
      reset();

      // Returning ABORT here stops the spacebar from activating. Alternatively,
      // we could remove this return statement, and instead allow the spacebar
      // to take effect, resulting in a space in the output.
      return EventHandlerResult::ABORT;

    } else if (event.key == Key_Backspace) {
      if (symbol_pos_ > 0)
        --symbol_pos_;

    } else {
      const char c = keyToChar(event.key);
      if (c)
        symbol_[symbol_pos_++] = c;
    }
  }

  return EventHandlerResult::OK;
}

} // namespace plugin

void eraseChars(int8_t n) {
  // For the `event.addr`, we could track the address of the Syster key, but it
  // might be on a layer that's no longer active by the time this gets
  // called. We could search the active keymap for an existing `Key_Backspace`,
  // but there might not be one. We could hijack the first idle key we find in
  // the keymap, but we probably don't need to. Even if some other plugin reacts
  // by inserting another event between these two, it's very unlikely that will
  // cause a user-visible error.
  auto event = KeyEvent(KeyAddr::none(), INJECTED, Key_Backspace);
  while (n >= 0) {
    event.state = IS_PRESSED | INJECTED;
    Runtime.handleKeyEvent(event);
    event.state = WAS_PRESSED | INJECTED;
    Runtime.handleKeyEvent(event);
  }
  Runtime.handleKeyEvent(event);
  // Change the event from a press to a release, but use the same id. This does
  // come with a small risk that another plugin will be tracking events, but
  // also ignoring event ids that it has seen before, but it's more likely to
  // avoid an error than to cause one.
  event.state = WAS_PRESSED | INJECTED;
  Runtime.handleKeyEvent(event);
}

} // namespace kaleidoscope


__attribute__((weak)) const char keyToChar(Key key) {
  if (key.getFlags() != 0)
    return 0;

  switch (key.getKeyCode()) {
  case Key_A.getKeyCode() ... Key_Z.getKeyCode():
    return 'a' + (key.getKeyCode() - Key_A.getKeyCode());
  case Key_1.getKeyCode() ... Key_0.getKeyCode():
    return '1' + (key.getKeyCode() - Key_1.getKeyCode());
  }

  return 0;
}

__attribute__((weak)) void systerAction(kaleidoscope::plugin::Syster::action_t action, const char *symbol) {
}

kaleidoscope::plugin::Syster Syster;
