/* Kaleidoscope-Macros - Macro keys for Kaleidoscope.
 * Copyright (C) 2017-2018  Keyboard.io, Inc.
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

#include "kaleidoscope/plugin/Macros/MacroKeyDefs.h"
#include "kaleidoscope/plugin/Macros/MacroSteps.h"
#include "kaleidoscope/keyswitch_state.h"
#include "kaleidoscope/key_events.h"

// Define this function in a Kaleidoscope sketch in order to trigger Macros.
const macro_t* macroAction(uint8_t macro_id, KeyEvent &event);

#ifndef NDEPRECATED
DEPRECATED(MACRO_ACTION_FUNCTION_V1)
const macro_t* macroAction(uint8_t macro_id, uint8_t key_state);

struct MacroKeyEvent {
  byte key_code;
  byte key_id;
  byte key_state;
};
#endif

// The number of simultaneously-active `Key` values that a macro can have
// running during a call to `Macros.play()`. I don't know if it's actually
// possible to override this by defining it in a sketch before including
// "Kaleidoscope-Macros.h", but probably not.
#if !defined(MAX_CONCURRENT_MACRO_KEYS)
#define MAX_CONCURRENT_MACRO_KEYS 8
#endif

namespace kaleidoscope {
namespace plugin {

class Macros : public kaleidoscope::Plugin {
 public:

  /// Send a key press event from a Macro
  ///
  /// Generates a new `KeyEvent` and calls `Runtime.handleKeyEvent()` with the
  /// specified `key`, then stores that `key` in an array of active macro key
  /// values. This allows the macro to press one key and keep it active when a
  /// subsequent key event is sent as part of the same macro sequence.
  void press(Key key);

  /// Send a key release event from a Macro
  ///
  /// Generates a new `KeyEvent` and calls `Runtime.handleKeyEvent()` with the
  /// specified `key`, then removes that key from the array of active macro
  /// keys (see `Macros.press()`).
  void release(Key key);

  /// Send a key "tap event" from a Macro
  ///
  /// Generates two new `KeyEvent` objects, one each to press and release the
  /// specified `key`, passing both in sequence to `Runtime.handleKeyEvent()`.
  void tap(Key key) const;

  /// Play a macro sequence of key events
  void play(const macro_t* macro_ptr);

  // Templates provide a `type()` function that takes a variable number of
  // `char*` (string) arguments, in the form of a list of strings stored in
  // PROGMEM, of the form `Macros.type(PSTR("Hello "), PSTR("world!"))`.
  inline const macro_t* type() const {
    return MACRO_NONE;
  }
  const macro_t* type(const char* string) const;
  template <typename... Strings>
  const macro_t* type(const char* first, Strings&&... strings) const {
    type(first);
    return type(strings...);
  }

  // ---------------------------------------------------------------------------
  // Event handlers
  EventHandlerResult onKeyEvent(KeyEvent &event);
  EventHandlerResult beforeReportingState(const KeyEvent &event);

 private:
  // An array of key values that are active while a macro sequence is playing
  static Key active_macro_keys_[MAX_CONCURRENT_MACRO_KEYS];

  // Set all keys in `active_macro_keys_` to Key_NoKey
  void clearActiveMacroKeys();

  // Translate and ASCII character value to a corresponding `Key`
  Key lookupAsciiCode(uint8_t ascii_code) const;

  // Test for a key that encodes a macro ID
  bool isMacrosKey(Key key) const {
    if (key >= ranges::MACRO_FIRST && key <= ranges::MACRO_LAST)
      return true;
    return false;
  }

#ifndef NDEPRECATED
 public:
  DEPRECATED(MACROS_ACTIVE_MACROS)
  static MacroKeyEvent active_macros[0];
  DEPRECATED(MACROS_ACTIVE_MACRO_COUNT)
  static uint8_t active_macro_count;
  DEPRECATED(MACROS_ADD_ACTIVE_MACRO_KEY)
  static void addActiveMacroKey(uint8_t macro_id, KeyAddr key_addr, uint8_t key_state) {}
  DEPRECATED(MACROS_KEY_ADDR)
  static KeyAddr key_addr;
#endif
};

} // namespace plugin
} // namespace kaleidosocpe

extern kaleidoscope::plugin::Macros Macros;
