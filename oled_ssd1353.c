
#include <asf.h>
#include "init.h"

#include "oled_ssd1353.h"

inline void port_in(void) {
	*((uint16_t*)PORT+0x04 ) = 0x0000ffff; // input
}

inline void port_out(void) {
	*((uint16_t*)PORT+0x08 ) = 0x0000ffff; // output
}

inline void port_wr(uint16_t data) {
	//*((uint16_t*)PORT_IOBUS+0x10 ) = data;
	*(uint16_t*)(((uint8_t*)PORT_IOBUS)+0x10) = data ;
}

inline uint16_t port_rd(void) {
	return *(uint16_t*)(((uint8_t*)PORT_IOBUS)+0x20); //*((uint16_t*)PORT_IOBUS+0x20 ) ;
}

 //uint16_t color565(uint8_t r, uint8_t g, uint8_t b)
//{
	//return (r << 11) | (g << 5) | b;
//}

void oled_init(void) {
	port_pin_set_output_level(V2_8V_OFF, 0);
	delay_us(250);
	port_pin_set_output_level(OLED_RST, 0);
	delay_us(250);
	port_pin_set_output_level(OLED_RST, 1);
	delay_us(250);
	
	port_pin_set_output_level(V18_ON, 1);
	delay_ms(250);
	
	oled_cmd(0xfd);
	oled_data_wr(0x12);
	
	oled_cmd(0xe2); //sw reset
	delay_ms(100);
	
	oled_cmd(0xae); //Display off
	oled_cmd(0xa8); //Set MUX ratio
	oled_data_wr(0x7f);
	oled_cmd(0xa1); //Display start line
	oled_data_wr(0x00);
	oled_cmd(0xa2); //Set display offset
	oled_data_wr(0x00);
	oled_cmd(0xa4); //Normal display
	oled_cmd(0xa0); //Set re-map
	oled_data_wr(0x64);
	oled_cmd(0x87); //Master current control
	oled_data_wr(0x0f);
	oled_cmd(0x81); //Set contrast level for R
	oled_data_wr(0x75); //Red contrast set
	oled_cmd(0x82); //Set contrast level for G
	oled_data_wr(0x60); //Green contrast set
	oled_cmd(0x83); //Set contrast level for B
	oled_data_wr(0x6a); //Blue contrast set
	oled_cmd(0xb1); //Phase adjust
	oled_data_wr(0x22);
	oled_cmd(0xb3); //Set frame rate
	oled_data_wr(0x40);
	oled_cmd(0xbb); //Set Pre-charge level
	oled_data_wr(0x08);
	oled_cmd(0xbe); //VCOMH setting
	oled_data_wr(0x3c);
	oled_cmd(0xb9);
	oled_cmd(0xaf); //Display on
	
	// set area
	
	oled_cmd(0x15); //Set Column Address
	 //Column Start Address
	 //Column End Address
	oled_data_wr(0x00) ;
	oled_data_wr(0x9f) ;
	
	oled_cmd(0x75); //Set Row Address
	 //Row Start Address
	 //Row End Address
	oled_data_wr(0x00) ;
	oled_data_wr(0x7f) ;
}

void oled_cmd(uint8_t data ) {
	
	port_pin_set_output_level(OLED_DC, 0);
	//nop_cmd();
	port_pin_set_output_level(OLED_CS, 0);
	//nop_cmd();
	
	//nop_cmd();
	port_pin_set_output_level(OLED_WR, 0);
	port_out();
	port_wr((uint16_t)data);
	//nop_cmd();
	//nop_cmd();
	//nop_cmd();
	port_pin_set_output_level(OLED_WR, 1);
	//nop_cmd();
	//nop_cmd();
	port_pin_set_output_level(OLED_CS, 1);
}

void oled_data_wr(uint16_t data ) {
	
	port_pin_set_output_level(OLED_DC, 1);
	//nop_cmd();
	port_pin_set_output_level(OLED_CS, 0);
	//nop_cmd();
	
	//nop_cmd();
	port_pin_set_output_level(OLED_WR, 0);
	port_out();
	port_wr(data);
	//nop_cmd();
	//nop_cmd();
	//nop_cmd();
	port_pin_set_output_level(OLED_WR, 1);
	//nop_cmd();
	//nop_cmd();
	port_pin_set_output_level(OLED_CS, 1);
}

void oled_data_burst_wr(uint16_t data ) {
		port_pin_set_output_level(OLED_WR, 0);
		port_wr(data);
		port_pin_set_output_level(OLED_WR, 1);
}

void oled_data_burst_start(void) {
	port_out();
	port_pin_set_output_level(OLED_DC, 1);
	port_pin_set_output_level(OLED_CS, 0);
}

void oled_data_burst_end(void) {
	port_pin_set_output_level(OLED_CS, 1);
}

uint16_t oled_data_rd(void) {
	port_pin_set_output_level(OLED_CS, 0);
	port_pin_set_output_level(OLED_DC, 1);
	
	port_pin_set_output_level(OLED_RD, 0);
	port_in();
	nop_cmd();
	nop_cmd();
	nop_cmd();
	nop_cmd();
	nop_cmd();
	uint16_t data = port_rd();
	
	port_pin_set_output_level(OLED_RD, 1);
	nop_cmd();
	nop_cmd();
	port_pin_set_output_level(OLED_CS, 1);
	return data ;
}