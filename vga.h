#include "types.h"
#include "hardware/pio.h"
#include "vga.pio.h"

#ifdef XGA
#define VRAM_STRIDE	(PIX_XN / 32 + 1)
#else
#define VRAM_STRIDE	(PIX_XN / 8 + 1)
#endif
#define VRAM_N		(VRAM_STRIDE * PIX_YN)

extern u32 vram[VRAM_N];
extern volatile u8 vsync_count;

#ifdef __cplusplus
extern "C" {
#endif

void vga_init(PIO, PIO);

#ifdef __cplusplus
}
#endif
