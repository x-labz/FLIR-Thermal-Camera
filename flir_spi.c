#include "flir_spi.h"

 struct spi_config config_spi_master;
 struct spi_slave_inst_config slave_dev_config;

void init_spi(struct spi_module * spi_master_instance ,struct spi_slave_inst * slave_inst ) {
	/* Configure and initialize software device instance of peripheral slave */
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	slave_dev_config.ss_pin = FLIR_SLAVE_SELECT_PIN;
	spi_attach_slave(slave_inst, &slave_dev_config);
	/* Configure, initialize and enable SERCOM SPI module */
	spi_get_config_defaults(&config_spi_master);
	
	config_spi_master.mux_setting = SPI_SIGNAL_MUX_SETTING_C ;
	config_spi_master.pinmux_pad0 = PINMUX_PA22D_SERCOM5_PAD0 ; // mosi
	config_spi_master.pinmux_pad1 = PINMUX_PA23D_SERCOM5_PAD1 ; // clk
	config_spi_master.pinmux_pad2 = PINMUX_PB22D_SERCOM5_PAD2 ; // miso
	config_spi_master.pinmux_pad3 = PINMUX_UNUSED  ; 
	
	config_spi_master.mode_specific.master.baudrate = 4000000;
	config_spi_master.transfer_mode = SPI_TRANSFER_MODE_3 ;
	config_spi_master.generator_source = GCLK_GENERATOR_0 ;

	spi_init(spi_master_instance, SERCOM5 , &config_spi_master);
	spi_enable(spi_master_instance);

	

	// END SPI
}
