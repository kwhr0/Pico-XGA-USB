#include "lib.h"
#include "vga.h"
#include "usb.h"
#include "xprintf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

#define FONT_XN		8
#define FONT_YN		12
#define SCREEN_XN	(PIX_XN / FONT_XN)
#define SCREEN_YN	(PIX_YN / FONT_YN)

extern const u8 gFont[];

static u8 cursX, cursY, cursActive, curColor;

static void writeChar(u8 c) {
	if (c < 0x20) c = 0x20;
	u8 *sp = (u8 *)&gFont[FONT_YN * (c - 0x20)];
#ifdef XGA
	u8 *dp = (u8 *)&vram[VRAM_STRIDE * FONT_YN * cursY] + cursX;
	for (u8 y = 0; y < FONT_YN; y++) {
		*dp = *sp++;
		dp += 4 * VRAM_STRIDE;
	}
#else
	u32 *dp = &vram[VRAM_STRIDE * FONT_YN * cursY + cursX];
	for (u8 y = 0; y < FONT_YN; y++) {
		u8 f = *sp++;
		u32 d = 0;
		for (u8 x = 0; x < FONT_XN; x++) {
			if (f & 1) d |= curColor << (x << 2);
			f >>= 1;
		}
		*dp = d;
		dp += VRAM_STRIDE;
	}
#endif
}

static void cursor() {
	if (!cursActive) return;
#ifdef XGA
	u8 *dp = (u8 *)&vram[VRAM_STRIDE * (FONT_YN * cursY)] + cursX;
	for (u8 y = 0; y < FONT_YN; y++) {
		*dp = ~*dp;
		dp += 4 * VRAM_STRIDE;
	}
#else
	u32 *dp = &vram[VRAM_STRIDE * FONT_YN * cursY + cursX];
	for (u8 y = 0; y < FONT_YN; y++) {
		*dp = ~*dp;
		dp += VRAM_STRIDE;
	}
#endif
}

static void newLine() {
	cursX = 0;
	if (++cursY >= SCREEN_YN) {
		--cursY;
		memcpy(vram, vram + VRAM_STRIDE * FONT_YN, 4 * (VRAM_N - VRAM_STRIDE * FONT_YN));
		memset(vram + VRAM_STRIDE * FONT_YN * (SCREEN_YN - 1), 0, 4 * VRAM_STRIDE * FONT_YN);
	}
	cursor();
}

static void xputchar(int c) {
	if (c == '\n') {
		cursor();
		newLine();
	}
	else {
		writeChar(c);
		if (++cursX >= SCREEN_XN) newLine();
		else cursor();
	}
}

void cls() {
	memset(vram, 0, 4 * VRAM_STRIDE * PIX_YN);
	cursX = cursY = 0;
}

void color(u8 c) {
	curColor = c & 7;
}

void cursorOn() {
	u8 l = cursActive;
	cursActive = 1;
	if (!l) cursor();
}

void cursorOff() {
	if (cursActive) cursor();
	cursActive = 0;
}

// keyboard

static const u8 normal[] = 
	"\0\0\0\0abcdefghijklmnopqrstuvwxyz12"
	"34567890\n\x1b\b\t -^@[\\];:\0,./\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\\\0\\\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

static const u8 shift[] = 
	"\0\0\0\0ABCDEFGHIJKLMNOPQRSTUVWXYZ!\""
	"#$%&\'()\0\n\x1b\b\t =~`{|}+*\0<>?\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0_\0|\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

u8 keyDown() {
	u8 usage, mod;
	if (keyget(&usage, &mod)) return mod & 0x22 ? shift[usage] : normal[usage];
	return 0;
}

void lib_init() {
	curColor = 7;
	xdev_out(xputchar);
}

// graphics

void point(u16 x, u16 y) {
	if (x < PIX_XN && y < PIX_YN) {
#ifdef XGA
		u32 *p = &vram[(x >> 5) + VRAM_STRIDE * y];
		u32 m = 1 << (x & 0x1f);
		if (curColor) *p |= m;
		else *p &= ~m;
#else
		u32 *p = &vram[(x >> 3) + VRAM_STRIDE * y];
		u32 s = (x & 7) << 2;
		*p = *p & ~(0xf << s) | curColor << s;
#endif
	}
}

void line(u16 x0, u16 y0, u16 x1, u16 y1) {
	u16 x = x0, y = y0, dx, dy, f;
	s8 sx, sy;
	s16 d;
	if (x0 < x1) {
		dx = x1 - x0;
		sx = 1;
	}
	else {
		dx = x0 - x1;
		sx = -1;
	}
	if (y0 < y1) {
		dy = y1 - y0;
		sy = 1;
	}
	else {
		dy = y0 - y1;
		sy = -1;
	}
	if (dx > dy) {
		d = dx >> 1;
		do {
			point(x, y);
			if ((d += dy) >= dx) {
				d -= dx;
				y += sy;
			}
			f = x != x1;
			x += sx;
		} while(f);
	}
	else {
		d = dy >> 1;
		do {
			point(x, y);
			if ((d += dx) >= dy) {
				d -= dy;
				x += sx;
			}
			f = y != y1;
			y += sy;
		} while (f);
	}
}

void circle(u16 x0, u16 y0, u16 r) {
	u16 x, y;
	s16 f;
	if (!r) return;
	x = r;
	y = 0;
	f = (-r << 1) + 3;
	while (x >= y) {
		point(x0 + x, y0 + y);
		point(x0 - x, y0 + y);
		point(x0 + x, y0 - y);
		point(x0 - x, y0 - y);
		point(x0 + y, y0 + x);
		point(x0 - y, y0 + x);
		point(x0 + y, y0 - x);
		point(x0 - y, y0 - x);
		if (f >= 0) {
			x--;
			f -= x << 2;
		}
		y++;
		f += (y << 2) + 2;
	}
}
