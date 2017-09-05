#include "i2c.h"


void configure_i2c(void)
{
	i2c_packet.address     = SLAVE_ADDRESS;
	i2c_packet.ten_bit_address = false;
	i2c_packet.high_speed      = false;
	i2c_packet.hs_master_code  = 0x0;
	
	
	/* Initialize config structure and software module */
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	/* Change buffer timeout to something longer */
	config_i2c_master.buffer_timeout = 65535;
	config_i2c_master.baud_rate = 200;
	/* Initialize and enable device with config */
	config_i2c_master.pinmux_pad0 = PINMUX_PA16C_SERCOM1_PAD0; //PINMUX_PA22C_SERCOM3_PAD0; // SDA
	config_i2c_master.pinmux_pad1 = PINMUX_PA17C_SERCOM1_PAD1 ;//PINMUX_PA23C_SERCOM3_PAD1 ; // SCL

	while(i2c_master_init(&i2c_master_instance, SERCOM1, &config_i2c_master) != STATUS_OK);
	i2c_master_enable(&i2c_master_instance);
}


static void i2c_write_complete_callback(void) {
	i2c_complete = true;
}

void configure_i2c_callbacks(void)
{
	/* Register callback function. */
	i2c_master_register_callback(&i2c_master_instance, i2c_write_complete_callback,
	I2C_MASTER_CALLBACK_READ_COMPLETE);
	i2c_master_enable_callback(&i2c_master_instance,
	I2C_MASTER_CALLBACK_READ_COMPLETE);
}