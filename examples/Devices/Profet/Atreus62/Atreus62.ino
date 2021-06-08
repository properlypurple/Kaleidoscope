#include "Kaleidoscope.h"
#include "Kaleidoscope-Macros.h"

enum { QWERTY};

/* *INDENT-OFF* */

KEYMAPS(
    [QWERTY] = KEYMAP(
 Key_Escape      ,Key_1         ,Key_2       ,Key_3             ,Key_4          ,Key_5                  ,Key_6     ,Key_7   ,Key_8          ,Key_9         ,Key_0          ,Key_Backspace
,Key_Tab         ,Key_Q         ,Key_W       ,Key_E             ,Key_R          ,Key_T                  ,Key_Y     ,Key_U   ,Key_I          ,Key_O         ,Key_P          ,Key_Backslash
,Key_Backspace   ,Key_A         ,Key_S       ,Key_D             ,Key_F          ,Key_G                  ,Key_H     ,Key_J   ,Key_K          ,Key_L         ,Key_Semicolon  ,Key_Quote
,Key_LeftShift   ,Key_Z         ,Key_X       ,Key_C             ,Key_V          ,Key_B       ,Key_Enter ,Key_N     ,Key_M   ,Key_Comma      ,Key_Period    ,Key_Slash      ,Key_LeftBracket
,Key_LeftControl ,Key_LeftGui   ,Key_LeftAlt ,___               ,Key_Backspace  ,Key_Delete  ,Key_Space ,Key_Space ,___     ,Key_LeftArrow  ,Key_UpArrow   ,Key_DownArrow  ,Key_RightArrow
),
);
/* *INDENT-ON* */


KALEIDOSCOPE_INIT_PLUGINS(Macros);

void setup() {
    Kaleidoscope.setup();
}

void loop() {
    Kaleidoscope.loop();
}
