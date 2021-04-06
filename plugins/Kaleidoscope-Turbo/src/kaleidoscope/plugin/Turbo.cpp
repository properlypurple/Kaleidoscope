/* -*- mode: c++ -*-
 * Kaleidoscope-Turbo
 * Copyright (C) 2018 ash lea
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

#include <Kaleidoscope-Turbo.h>
#include <Kaleidoscope-LEDControl.h>
#include "kaleidoscope/layers.h"
#include "kaleidoscope/keyswitch_state.h"

namespace kaleidoscope {
namespace plugin {

uint16_t Turbo::interval_ = 10;
uint16_t Turbo::flash_interval_ = 69;
bool Turbo::sticky_ = false;
bool Turbo::flash_ = true;
cRGB Turbo::active_color_ = CRGB(160, 0, 0);

bool Turbo::active_ = false;
uint32_t Turbo::start_time_ = 0;
uint32_t Turbo::flash_start_time_ = 0;

uint16_t Turbo::interval() {
  return interval_;
}
void Turbo::interval(uint16_t newVal) {
  interval_ = newVal;
}

uint16_t Turbo::flashInterval() {
  return flash_interval_;
}
void Turbo::flashInterval(uint16_t newVal) {
  flash_interval_ = newVal;
}

bool Turbo::sticky() {
  return sticky_;
}
void Turbo::sticky(bool newVal) {
  sticky_ = newVal;
}

bool Turbo::flash() {
  return flash_;
}
void Turbo::flash(bool newVal) {
  flash_ = newVal;
}

cRGB Turbo::activeColor() {
  return active_color_;
}
void Turbo::activeColor(cRGB newVal) {
  active_color_ = newVal;
}

EventHandlerResult Turbo::onKeyEvent(KeyEvent &event) {
  if (event.key != Key_Turbo)
    return EventHandlerResult::OK;

  if (keyToggledOn(event.state)) {
    active_ = true;
  } else {
    active_ = false;
  }
  return EventHandlerResult::EVENT_CONSUMED;
}

EventHandlerResult Turbo::afterEachCycle() {
  if (active_) {
    if (Runtime.hasTimeExpired(start_time_, interval_)) {
      // clear report, and send it, then restore it
      Runtime.hid().keyboard().releaseAllKeys();
      Runtime.hid().keyboard().sendReport();
      // The HID module doesn't provide a way to simply send a blank report,
      // then restore the previous one, so we use `handleKeyEvent()` to restore
      // it, hopefully back to its original state.
      Runtime.handleKeyEvent(KeyEvent(KeyAddr::none(), IS_PRESSED | INJECTED, Key_NoKey));
      start_time_ = Runtime.millisAtCycleStart();

      // Just in case we miss a key toggled off event, check every repeat cycle,
      // and if there's no turbo key active, shut off.
      active_ = false;
      for (Key key : live_keys.all()) {
        if (key == Key_Turbo) {
          active_ = true;
        }
      }
    }
  }
  return EventHandlerResult::OK;
}

EventHandlerResult Turbo::beforeSyncingLeds() {
  if (flash_) {
    static bool leds_on = false;
    cRGB color = CRGB(0, 0, 0);
    if (leds_on) {
      color = active_color_;
    }
    if (Runtime.hasTimeExpired(flash_start_time_, flash_interval_)) {
      for (KeyAddr key_addr : KeyAddr::all()) {
        Key key = live_keys[key_addr];
        if (key.isKeyboardKey()) {
          LEDControl::setCrgbAt(key_addr, color);
        }
      }
      flash_start_time_ = Runtime.millisAtCycleStart();
      leds_on = !leds_on;
    }
  }
  return EventHandlerResult::OK;
}

}
}

kaleidoscope::plugin::Turbo Turbo;
