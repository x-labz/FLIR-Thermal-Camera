#include "init.h"
#include "oled_ssd1353.h"
#include "pic.h"

void demo(void) {
	int16_t offset=0, dir =1 ;
	
	while (1)
	{
		oled_cmd(0x5c);
		
		oled_data_burst_start();
		
		int read_pointer = offset + 127 * 320 + 159 ;
		
		for (int rw = 127; rw >= 0; rw--) {
			for (int pix = 159; pix >= 0; pix--) {
				//oled_data_burst_wr(image_data_oled1[read_pointer]) ;
				//read_pointer--;
				read_pointer =  rw * 320 + pix + offset ;
				oled_data_burst_wr(image_data_oled1[read_pointer]) ;
			}
			read_pointer-=160;
		}
		
		oled_data_burst_end();
		
		offset+=dir*1;
		if (offset > 160) {
			dir = -1;
			offset = 160;
		}
		if (offset < 0 ) {
			dir = 1;
			offset = 0;
		}
	}
	
}