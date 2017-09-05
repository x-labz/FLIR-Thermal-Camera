

#ifndef OLED_SSD1353_H_
#define OLED_SSD1353_H_

#include <asf.h>

inline void port_in(void);
inline void port_out(void) ;
inline void port_wr(uint16_t ) ;
inline uint16_t port_rd(void);

 //uint16_t color565(uint8_t , uint8_t , uint8_t );
 inline uint16_t color565(uint8_t r, uint8_t g, uint8_t b)  {
	  return ((r<<8) & 0b1111100000000000 ) | ((g << 3) & 0b0000011111100000 ) + ( ( b >>3) & 0b0000000000011111 ) ;
  }

#define COLOR_GRAY 0x9492
#define COLOR_DARKGRAY 0x630c
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xffff
#define COLOR_RED 0xf800
#define COLOR_BLUE 0x001f
#define COLOR_GREEN 0x07e0

void oled_init(void);

void oled_cmd(uint8_t);
void oled_data_wr(uint16_t  );
uint16_t oled_data_rd(void);

void oled_data_burst_wr(uint16_t  );
void oled_data_burst_start(void);
void oled_data_burst_end(void) ;

#endif /* OLED_SSD1353_H_ */