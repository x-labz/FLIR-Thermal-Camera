#include <asf.h>
#include "bmp.h"


FRESULT write_bmp( uint16_t* image,  uint16_t* palette, FIL* file_object_p ) {
	
	FRESULT result ;
	
	uint32_t bytes_written;
	
	uint8_t file[14] = {
		'B','M', // magic
		0,0,0,0, // size in bytes
		0,0, // app data
		0,0, // app data
		40+14,0,0,0 // start of data offset
	};
	uint8_t info[40] = {
		40,0,0,0, // info hd size
		0,0,0,0, // width
		0,0,0,0, // heigth
		1,0, // number color planes
		24,0, // bits per pixel
		0,0,0,0, // compression is none
		0,0,0,0, // image bits size
		0x13,0x0B,0,0, // horz resoluition in pixel / m
		0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
		0,0,0,0, // #colors in pallete
		0,0,0,0 // #important colors
	};

	uint32_t padSize  = (4-(WIDTH*3)%4)%4;   // 80*60 => 0
	uint32_t sizeData = WIDTH*HEIGHT*3 + HEIGHT*padSize;
	uint32_t sizeAll  = sizeData + sizeof(file) + sizeof(info);

	file[ 2] = (uint8_t)( sizeAll    );
	file[ 3] = (uint8_t)( sizeAll>> 8);
	file[ 4] = (uint8_t)( sizeAll>>16);
	file[ 5] = (uint8_t)( sizeAll>>24);

	info[ 4] = (uint8_t)( WIDTH   );
	info[ 5] = (uint8_t)( WIDTH>> 8);
	info[ 6] = (uint8_t)( WIDTH>>16);
	info[ 7] = (uint8_t)( WIDTH>>24);

	info[ 8] = (uint8_t)( HEIGHT    );
	info[ 9] = (uint8_t)( HEIGHT>> 8);
	info[10] = (uint8_t)( HEIGHT>>16);
	info[11] = (uint8_t)( HEIGHT>>24);

	info[20] = (uint8_t)( sizeData    );
	info[21] = (uint8_t)( sizeData>> 8);
	info[22] = (uint8_t)( sizeData>>16);
	info[23] = (uint8_t)( sizeData>>24);

	result = f_write(file_object_p,file,sizeof(file), &bytes_written);
	if (result != FR_OK) return result ;
	
	result = f_write(file_object_p,info,sizeof(info), &bytes_written);
	if (result != FR_OK) return result ;

	uint8_t pad[3] = {0,0,0};

	for ( int16_t y=HEIGHT-1; y>=0; y-- )
	{
		uint8_t data[3*WIDTH];
		for ( uint16_t x=0; x<WIDTH; x++ )
		{
			uint16_t pix = image[WIDTH*y + x] ;
		
			uint16_t val = palette[pix] ;
			
			data[x*3+2] = (val & 0b1111100000000000) >> 8 ; 
			data[x*3+1] = (val & 0b0000011111100000) >> 3 ;
			data[x*3+0] = (val & 0b0000000000011111) << 3 ;
		}
		result = f_write(file_object_p,data,3*WIDTH, &bytes_written);
		if (result != FR_OK) return result ;

		//result = f_write(file_object_p,pad,padSize, &bytes_written);   // PADDING
		//if (result != FR_OK) return result ;
	}
	
	return result ;
}

void test_bmp(uint16_t* image,uint16_t* palette) {
	//for (uint16_t y=0; y!=60; y++)
	//{
		//for (uint16_t x=0; x!=80; x++) {
		//image[y*80+x] = 0;
		//}
	//}
	
	//for (uint16_t y=0; y!=60; y++)
	//{
		//image[y*80+4] = 1;
		//image[y*80+32] = 2;
	//}
	//for (uint16_t x=0; x!=80; x++) {
		//image[5*80+x] = 3;
		//image[24*80+x] = 1;
	//}
	
	for (uint16_t i = 0; i!= 210;i++ ) {
		image[i] = 0;
	}
	for (uint16_t i = 0; i!= (4800-210) ;i++ ) {
		image[i+210] = i/18 ;
	}
}