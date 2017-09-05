

#include <asf.h>
#include "flir_spi.h"
#include "flir-dma.h"



DmacDescriptor dma_descriptor_tx;
DmacDescriptor dma_descriptor_rx;

 extern  uint8_t data[164] ;
 extern  uint8_t dummy[2];

 void transfer_tx_done(struct dma_resource* const resource ) {
	transfer_tx_is_done = true;
}

 void transfer_rx_done(struct dma_resource* const resource ) {
	transfer_rx_is_done = true;
}

///////////////////////////////////////////////////////////////////////

 void configure_dma_resource_tx(struct dma_resource *tx_resource) {
	struct dma_resource_config tx_config;

	dma_get_config_defaults(&tx_config);
	tx_config.peripheral_trigger = SERCOM5_DMAC_ID_TX;
	tx_config.trigger_action = DMA_TRIGGER_ACTON_BEAT;
	tx_config.priority = DMA_PRIORITY_LEVEL_0;
	dma_allocate(tx_resource, &tx_config);
}

 void configure_dma_resource_rx(struct dma_resource *rx_resource) {
	struct dma_resource_config rx_config;

	dma_get_config_defaults(&rx_config);
	rx_config.peripheral_trigger = SERCOM5_DMAC_ID_RX;
	rx_config.trigger_action = DMA_TRIGGER_ACTON_BEAT;
	rx_config.priority = DMA_PRIORITY_LEVEL_1;
	dma_allocate(rx_resource, &rx_config);
}

///////////////////////////////////////////////////////////////////////

 void setup_transfer_descriptor_tx(DmacDescriptor *tx_descriptor) {
	struct dma_descriptor_config tx_descriptor_config;

	dma_descriptor_get_config_defaults(&tx_descriptor_config);
	tx_descriptor_config.beat_size = DMA_BEAT_SIZE_BYTE;
	tx_descriptor_config.dst_increment_enable = false;
	tx_descriptor_config.src_increment_enable = false;
	tx_descriptor_config.block_transfer_count = 164 ;//sizeof(buffer_tx)/sizeof(uint8_t);
	tx_descriptor_config.source_address = (uint32_t)dummy ; //+ 164; //(uint32_t)buffer_tx + sizeof(buffer_tx);
	tx_descriptor_config.destination_address = (uint32_t)(&spi_master_instance.hw->SPI.DATA.reg);
	dma_descriptor_create(tx_descriptor, &tx_descriptor_config);
}

 void setup_transfer_descriptor_rx(DmacDescriptor *rx_descriptor) {
	struct dma_descriptor_config rx_descriptor_config;

	dma_descriptor_get_config_defaults(&rx_descriptor_config);
	rx_descriptor_config.beat_size = DMA_BEAT_SIZE_BYTE;
	rx_descriptor_config.src_increment_enable = false;
	rx_descriptor_config.block_transfer_count = 164 ; //sizeof(buffer_rx)/sizeof(uint8_t);
	rx_descriptor_config.source_address = (uint32_t)(&spi_master_instance.hw->SPI.DATA.reg);
	rx_descriptor_config.destination_address = (uint32_t)data + 164; //(uint32_t)buffer_rx + sizeof(buffer_rx);
	dma_descriptor_create(rx_descriptor, &rx_descriptor_config);
}

//////////////////////////////////////////////////////////////////////

void init_dma(void) {
	transfer_tx_is_done = false;
	transfer_rx_is_done = false ;
	
	configure_dma_resource_tx(&dma_resource_tx);
	configure_dma_resource_rx(&dma_resource_rx);

	setup_transfer_descriptor_tx(&dma_descriptor_tx);
	setup_transfer_descriptor_rx(&dma_descriptor_rx);

	dma_add_descriptor(&dma_resource_tx, &dma_descriptor_tx);
	dma_add_descriptor(&dma_resource_rx, &dma_descriptor_rx);

	dma_register_callback(&dma_resource_tx, transfer_tx_done,DMA_CALLBACK_TRANSFER_DONE);
	dma_register_callback(&dma_resource_rx, transfer_rx_done,DMA_CALLBACK_TRANSFER_DONE);

	dma_enable_callback(&dma_resource_tx, DMA_CALLBACK_TRANSFER_DONE);
	dma_enable_callback(&dma_resource_rx, DMA_CALLBACK_TRANSFER_DONE);
}