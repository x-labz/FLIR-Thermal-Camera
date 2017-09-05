

#ifndef SPI_H_
#define SPI_H_

#include <asf.h>

#define FLIR_SLAVE_SELECT_PIN PIN_PB23

struct spi_slave_inst slave_inst;
struct spi_module spi_master_instance;

//void callback_spi_master(const struct spi_module * const  );
void init_spi(struct spi_module * ,struct spi_slave_inst * ) ;

#endif /* SPI_H_ */