#include "types.h"

void lib_init();
void cls();
void cursorOn();
void cursorOff();

u8 keyDown();

void color(u8 c);
void point(u16 x, u16 y);
void line(u16 x0, u16 y0, u16 x1, u16 y1);
void circle(u16 x0, u16 y0, u16 r);
