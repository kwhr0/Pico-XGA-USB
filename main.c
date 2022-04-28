#include "vga.h"
#include "usb.h"
#include "lib.h"
#include "xprintf.h"
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"

static u16 R(u16 n) {
	return (u16)((u32)n * (rand() & 0x7fff) >> 15);
}

int main() {
#ifdef XGA
	set_sys_clock_khz(130000, true); // 130MHz
#else
	set_sys_clock_khz(126000, true); // 126MHz
#endif
	stdio_init_all();
	vga_init(pio0, pio1);
	usb_init(pio1);
	lib_init();
	srand(12345);
	while (1) {
		color(7);
		xputs("C: character\n");
		xputs("E: erase\n");
		xputs("G: graphic\n");
		xputs("K: key test\n");
		xputs("V: character (wait for VSync)\n");
		cursorOn();
		xprintf("select>");
		u8 c;
		while (!(c = keyDown())) rand();
		xprintf("%c\n", c);
		u8 i, v = 0;
		switch (c) {
		case 'v':
			v = 1;
			// no break
		case 'c':
			cursorOff();
			for (u8 *p = (u8 *)0x10000000; p < (u8 *)0x10200000; p += 0x10) {
				c = vsync_count;
				xprintf("%08X ", p);
				for (i = 0; i < 0x10; i++) xprintf("%02X ", p[i]);
				for (i = 0; i < 0x10; i++) xputc(p[i] >= ' ' ? p[i] : '.');
				while (v && c == vsync_count)
					;
				xputc('\n');
				if (keyDown()) break;
			}
			break;
		case 'e':
			cls();
			break;
		case 'g':
			for (i = 0; i < 100; i++) {
				color(R(7) + 1);
				line(R(PIX_XN), R(PIX_YN), R(PIX_XN), R(PIX_YN));
				color(R(7) + 1);
				circle(R(PIX_XN), R(PIX_YN), R(PIX_YN >> 1));
			}
			break;
		case 'k':
			xputs("Press [Esc] to exit.\n");
			do {
				c = keyDown();
				if (c >= ' ' || c == '\n') xputc(c);
				sleep_ms(1);
			} while (c != 0x1b);
			xputc('\n');
			break;
		default:
			xputs("?\n");
			break;
		}
	}
	return 0;
}
