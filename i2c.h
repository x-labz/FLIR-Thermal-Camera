

#ifndef I2C_H_
#define I2C_H_

#include <asf.h>

#define I2C_DATA_LENGTH 16
//static uint8_t buffer[I2C_DATA_LENGTH] ;

#define SLAVE_ADDRESS  0x2A

struct i2c_master_module i2c_master_instance;
struct i2c_master_packet i2c_packet;
static volatile bool i2c_complete = false;

//static void i2c_write_complete_callback(void);
void configure_i2c_callbacks(void);
void configure_i2c(void);


#endif /* I2C_H_ */