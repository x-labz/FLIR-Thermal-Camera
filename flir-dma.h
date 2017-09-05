

#ifndef FLIR-DMA_H_
#define FLIR-DMA_H_

struct dma_resource dma_resource_tx;
struct dma_resource dma_resource_rx;

volatile bool transfer_tx_is_done ;
volatile bool transfer_rx_is_done ;

void transfer_tx_done(struct dma_resource* const  ) ;
void transfer_rx_done(struct dma_resource* const  ) ;

void configure_dma_resource_tx(struct dma_resource *);
void configure_dma_resource_rx(struct dma_resource *);
void setup_transfer_descriptor_tx(DmacDescriptor *);
void setup_transfer_descriptor_rx(DmacDescriptor *);
void init_dma(void) ;

#endif /* FLIR-DMA_H_ */