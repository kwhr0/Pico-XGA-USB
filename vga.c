#include "vga.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

u32 vram[VRAM_N];
volatile u8 vsync_count;

static PIO pio;
static int dma_chan;

static void vsync_intr() {
	pio_interrupt_clear(pio, 0);
	dma_channel_set_read_addr(dma_chan, vram, true);
	vsync_count++;
}

void vga_init(PIO _pio, PIO piosub) {
	pio = _pio;
	uint sm_p = pio_claim_unused_sm(pio, true);
	pixel_program_init(pio, sm_p);
	uint sm_h = pio_claim_unused_sm(pio, true);
	hsync_program_init(pio, sm_h);
	uint sm_v = pio_claim_unused_sm(pio, true);
	vsync_program_init(pio, sm_v);
#ifdef PIN_RGB
	if (piosub) {
		uint offset = pio_add_program(piosub, &copy_program);
		for (u8 i = PIN_RGB; i < PIN_RGB + 3; i++) {
			uint sm = pio_claim_unused_sm(piosub, true);
			copy_program_init(piosub, sm, offset, i);
		}
	}
#endif
	dma_chan = dma_claim_unused_channel(true);
	dma_channel_config c = dma_channel_get_default_config(dma_chan);
	channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
	channel_config_set_read_increment(&c, true);
	channel_config_set_dreq(&c, pio_get_dreq(pio, sm_p, true));
	dma_channel_configure(dma_chan, &c, &pio0_hw->txf[sm_p], NULL, VRAM_N, false);
//
	pio_set_irq0_source_enabled(pio, pis_interrupt0, true);
	irq_set_exclusive_handler(PIO0_IRQ_0, vsync_intr);
	irq_set_enabled(PIO0_IRQ_0, true);
	pio_sm_set_enabled(pio, sm_p, true);
	pio_sm_set_enabled(pio, sm_h, true);
	pio_sm_set_enabled(pio, sm_v, true);
}
