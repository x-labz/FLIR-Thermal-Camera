

#include <asf.h>
#include <string.h>
#include <stdio.h>
#include "init.h"
#include "oled_ssd1353.h"
#include "buttons.h"
#include "demo.h"
#include "bmp.h"
#include "flir.h"
#include "LEPTON_SYS.h"
#include "LEPTON_AGC.h"
#include "flir_spi.h"
#include "i2c.h"
#include "font8.h"
#include "flir-dma.h"
#include "pac-man.h"

void actions(uint8_t) ;
void save_img_to_sd(void) ;
void port_test(void) ;
void oled_test(void);

#define STRING_COUNT 14
#define STRING_LENGTH 20
#define MENU_ITEMS 11

COMPILER_ALIGNED(16)

typedef struct
{
	char chars[STRING_LENGTH];
	uint8_t x;
	uint16_t color;
	uint16_t bg_color;
} text_type ;

typedef struct  {
	uint16_t image[80*60];
	uint16_t osd[8*160];
	text_type strings[STRING_COUNT] ;
} flir_data_type;

union {
	flir_data_type flir_data;
	uint8_t frame_buffer[160*128];
} data_storage;

uint16_t *image = data_storage.flir_data.image ;
uint16_t *osd = data_storage.flir_data.osd ;
text_type *strings = data_storage.flir_data.strings ;
uint8_t *pac_image = data_storage.frame_buffer ;

const uint16_t* palettes[2];
const uint16_t* flir_palette = color_palette ;

uint8_t oled_brightness = 1;
uint8_t ROI_value = 0;

//static uint16_t image[80*60  ] ; //      PACMAN
//static uint8_t image[160*128];
uint8_t data[164] ; //
uint8_t dummy[2];
//static uint16_t osd[8*160] ; //    PACMAN

volatile bool adc_read_done = false;
volatile bool sleepmode = false;
volatile bool wake_me_up = false;
volatile bool set_clock = false;

volatile app_mode = MODE_SETUP_DATETIME;
bool adc_in_progress = false;
uint16_t adc_result;

static volatile bool render = true;
bool shutdown = false;
int16_t shutdown_offset ;
int16_t shutdown_offset_x ;

text_type *string_adc_ref ;
text_type *string_batt_ref;

char pic_file_name[20] ;

volatile bool sw_agc = false;

int8_t adc_result_buffer[80] ;
uint8_t adc_pointer = 0;

int8_t selected_button = 0;

void clear_osd(void) {
	for (uint16_t i=0; i!=8*160; i++)	{
		osd[i] = 0x00;
	}
}

void render_osd(void) {
	clear_osd();
	for (uint8_t s=0; s!=STRING_COUNT; s++ ) {
		uint8_t c = 0;
		
		while (strings[s].chars[c] != 0) {
			uint8_t* p = map_8x8_array[strings[s].chars[c]-0x20].data ;
			for (uint8_t y=0; y !=8; y ++) {
				for (uint8_t x=0; x!=8; x++) {
					uint16_t color = *(p+y*8+x) == 0 ? strings[s].bg_color : strings[s].color ;
					osd[y*160+x+strings[s].x+c*8] = color ;
				}
			}
			c++;
		}
	}
	
}

void startADC(void) {
	if (app_mode!=MODE_SETUP_DATETIME && !adc_in_progress) {
		adc_enable(&adc_instance);
		
		port_pin_set_output_level(BATT_DIV_ON, 1);
		adc_read_buffer_job(&adc_instance,&adc_result,1) ;
		adc_in_progress = true;
	}
}

void initStrings(uint8_t mode) {
	string_adc_ref = &strings[STRING_COUNT-1];
	string_batt_ref = &strings[STRING_COUNT-2];
	
	for (uint8_t s=0; s!=STRING_COUNT; s++) {
		strings[s].color = COLOR_GRAY;  //color565(0x90,0x90,0x90) ;  //0b1111100000000000;
		strings[s].bg_color = 0x0000;
		for (uint8_t c = 0; c!= STRING_LENGTH ; c++) {
			strings[s].chars[c] = 0 ;
		}
	}
	
	if (app_mode != MODE_SETUP_DATETIME) {
		// battery - 0.
		// voltage: (3.65V) - 1-5.
		
		strings[0].x = 8*8;
		strings[0].chars[0] = 0x7f; // FFC
		
		strings[1].x = 9*8;
		strings[1].chars[0] = 0x80; // FFC cmd
		
		strings[2].x = 10*8;
		strings[2].chars[0] = 0x81; // disk
		
		strings[3].x = 11*8;
		strings[3].chars[0] = 0x83; // sleep
		
		strings[4].x = 12*8;
		strings[4].chars[0] = 0x82; // power off
		
		strings[5].x = 13*8 ;
		strings[5].chars[0] = 'P'; // palette
		
		strings[6].x = 14*8 ;
		strings[6].chars[0] = 0x8b; // pacman
		
		strings[7].x = 15*8 ;
		strings[7].chars[0] = 0x8c; // brightness
		
		strings[8].x = 16*8 ;
		strings[8].chars[0] = 0x8d; // ROI
		
		strings[9].x = 17*8 ;
		strings[9].chars[0] = 0x8e; // Replay
		
		strings[10].x = 18*8 ;
		strings[10].chars[0] = sw_agc ? 0x8f : 0x90; // AGC
		
		strings[11].x = 19*8 ;
		strings[11].chars[0] = 0x91; // BATT. chart
		
		string_batt_ref->x = 0*8 ;
		string_batt_ref->chars[0] = 0x84;  // battery
		
		string_adc_ref->x = 1*8; // ADC result
		
		for (uint8_t i=0; i!= STRING_LENGTH; i++)
		{
			string_adc_ref->chars[i] = 0;
		}
	}
}

int main (void)
{
	palettes[0] = color_palette ;
	palettes[1] = green_palette ;
	
	initStrings(MODE_SETUP_DATETIME);
	
	for (uint8_t i=0; i!= 80; i++)
	{
		adc_result_buffer[i] = 0;
	}
	
	system_init();
	irq_initialize_vectors();
	delay_init();
	init_board();
	
	rtc_calendar_setup(); // + ALARM
	
	init_spi(&spi_master_instance,&slave_inst);
	
	init_dma();

	configure_i2c();
	
	system_interrupt_enable_global();
	
	flir_setup();
	oled_init();
	
	//pacman(pac_image);
	
	// -------  TEST  -----------
	//port_test();
	//sd_test();
	//oled_test();
	
	
	// -------------------  MAIN loop

	
	
	int8_t value_change_dir = 0;
	
	static volatile uint32_t frame_id_1 = 12345678,	frame_id ;
	uint8_t frame_p=0;
	//static uint32_t frames[16];
	static uint8_t line_id = 0x00;
	//static uint8_t lines[64];
	
	uint16_t *image_p ;
	static volatile bool getFrame = false;
	
	static volatile bool resync = false;
	static  uint8_t volatile spi_error ;
	static int16_t pix;
	static uint16_t color;
	uint8_t pixel = 0xff;
	static volatile uint16_t spi_reads = 0;
	uint16_t min_val, max_val ;
	uint32_t avg_value;
	
	bool toggle_char = false;
	
	uint32_t prev_time_minute = 0;
	uint32_t render_counter = 0;
	
	for (uint16_t p=0; p!=4800; p++) {
		image[p] = 10;
	}
	
	startADC() ;
	
	struct rtc_calendar_time time;
	rtc_calendar_get_time_defaults(&time);
	time.year   = 2017;
	time.month  = 5;
	time.day    = 16;
	time.hour   = 10;
	time.minute = 14;
	time.second = 00;
	rtc_calendar_set_time(&rtc_instance, &time);
	
	for(;;) {
		
		//  ---BUTTONS---
		get_buttons(&buttons_state);
		
		switch (app_mode) {
			case MODE_CAMERA: {
				if (buttons_state.button_left == false &&  buttons_state.button_left != buttons_state.button_left_1 ) {
					selected_button--;
				}
				if (selected_button<0) selected_button = MENU_ITEMS ;
				if (buttons_state.button_right == false && buttons_state.button_right != buttons_state.button_right_1 ) {
					selected_button++;
				}
				if (selected_button > MENU_ITEMS) selected_button = 0;
				if (buttons_state.button_up == false && buttons_state.button_up != buttons_state.button_up_1) {
					actions(selected_button) ;
					//selected_button = 0;
				}
				for (uint8_t i=0; i!= MENU_ITEMS; i++) strings[i+1].color = COLOR_GRAY;
				
				if (selected_button > 0 ) {
					strings[selected_button].color = COLOR_GREEN;
				}
				if (port_pin_get_input_level(SD_CARD_DETECT) == true) {
					strings[2].color = COLOR_RED;
				}
			}
			break;
			
			case MODE_CHART: {
				if (buttons_state.button_down == false && buttons_state.button_down != buttons_state.button_down_1) {
					app_mode = MODE_CAMERA ;
				}
			}
			break;
			
			case MODE_SETUP_DATETIME: {
				if (buttons_state.button_left == false &&  buttons_state.button_left != buttons_state.button_left_1 ) {
					selected_button--;
					if (selected_button == 2 || selected_button == 5 || selected_button == 8 || selected_button == 11) selected_button--;
				}
				if (buttons_state.button_right == false && buttons_state.button_right != buttons_state.button_right_1 ) {
					selected_button++;
					if (selected_button == 2 || selected_button == 5 || selected_button == 8 || selected_button == 11) selected_button++;
				}
				if (selected_button < 0) selected_button = 15;
				if (selected_button > 15) selected_button = 0 ;
				
				
				value_change_dir = 0;
				if (buttons_state.button_up == false && buttons_state.button_up != buttons_state.button_up_1) {
					value_change_dir = -1;
				}
				if (buttons_state.button_down == false && buttons_state.button_down != buttons_state.button_down_1) {
					value_change_dir = 1;
				}
				
				
				switch (selected_button) {
					case 0:
					time.year+= 10*value_change_dir ;
					break;
					
					case 1:
					time.year+= value_change_dir ;
					break;
					
					case 3:
					time.month+= 10 * value_change_dir ;
					if (time.month < 1) time.month = 1;
					if (time.month > 12) time.month = 12;
					break;
					
					case 4:
					time.month+= value_change_dir ;
					if (time.month < 1) time.month = 1;
					if (time.month > 12) time.month = 12;
					break;
					
					case 6:
					time.day+= 10 * value_change_dir ;
					if (time.day < 1) time.day = 1;
					if (time.day > 31) time.day = 31;
					break;
					
					case 7:
					time.day+= value_change_dir ;
					if (time.day < 1) time.day = 1;
					if (time.day > 31) time.day = 31;
					break;
					
					case 9:
					time.hour+= 10* value_change_dir;
					if (time.hour < 0) time.hour = 0;
					if (time.hour > 23) time.hour = 23;
					break;
					
					case 10:
					time.hour+=  value_change_dir;
					if (time.hour < 0) time.hour = 0;
					if (time.hour > 23) time.hour = 23;
					break;
					
					case 12:
					time.minute+= 10*value_change_dir;
					if (time.minute < 0) time.minute = 0;
					if (time.minute > 59) time.minute = 59;
					break;
					
					case 13:
					time.minute+= value_change_dir;
					if (time.minute < 0) time.minute = 0;
					if (time.minute > 59) time.minute = 59;
					break;
					
					case 14:
					if (value_change_dir != 0) {
						rtc_calendar_set_time(&rtc_instance, &time);
						
						alarm.mask = RTC_CALENDAR_ALARM_MASK_SEC;
						alarm.time.year = time.year;
						alarm.time.month = time.month;
						alarm.time.day = time.day;
						alarm.time.hour = time.hour;
						alarm.time.minute = time.minute;
						
						alarm.time.second = 10;
						rtc_calendar_set_alarm(&rtc_instance, &alarm, RTC_CALENDAR_ALARM_0);
						
						app_mode = MODE_CAMERA;
						selected_button = 0;
						
						initStrings( MODE_CAMERA );
						
						startADC();
					}
					break;
				}
				if (app_mode == MODE_SETUP_DATETIME) {
					strings[0].x = 0;
					sprintf(strings[0].chars, "%4d.%02d.%02d.%02d:%02d%c", time.year,time.month,time.day,time.hour,time.minute, 0x8a ) ;
					toggle_char = !toggle_char ;
					if (toggle_char) {
						strings[0].chars[2+selected_button] = 0x20;
					}
					delay_ms(25);
					render_osd();
					render = true;
				}
			}
			break;
		}
		

		
		//  --- ADC ---
		
		//struct rtc_calendar_time time;

		//rtc_calendar_get_time(&rtc_instance, &time) ;
		//uint32_t time ; // = rtc_instance.hw->MODE2.CLOCK.reg;                 //MODE2.CLOCK.reg;
		if (render_counter > (prev_time_minute+540 ) && !adc_in_progress) {
			startADC();
			prev_time_minute  = render_counter;
		}
		
		
		if (adc_read_done) {
			//uint16_t result ;
			//adc_read(&adc_instance,&result) ;
			port_pin_set_output_level(BATT_DIV_ON, 0);
			
			sprintf(string_adc_ref->chars,"%1.2fV",(((float)adc_result)/254.4) );
			
			int16_t batt = (adc_result-790) / 26 ;
			if (batt < 0) batt = 0;
			if (batt > 5) batt = 5;
			string_batt_ref->chars[0] = 0x84 +  batt ;
			string_batt_ref->color = adc_result < 790 ? COLOR_RED : COLOR_GRAY ;
			
			// 1024 - 4,02V
			// 664 - 2,61V
			// diff:  --> 60px  | 0,0235V / pix
			// 3V - 16pix
			// 3,5V - 38pix
			
			
			adc_result_buffer[adc_pointer] = (adc_result - 664) / 6 ;
			if (adc_result_buffer[adc_pointer] > 59 ) adc_result_buffer[adc_pointer] = 59;
			if (adc_result_buffer[adc_pointer] < 0 ) adc_result_buffer[adc_pointer] = 0;
			
			adc_read_done = false;
			adc_in_progress = false;
			
			adc_disable(&adc_instance);
		}
		
		if (wake_me_up) {
			wake_me_up = false;
			sleepmode = false;
			selected_button = 0;
			startADC();
			actions(ACTION_WAKE_UP);
		}
		
		if (sleepmode && !adc_in_progress) {
		//if ( sleepmode ) {			 
			system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
			system_sleep();			
		}
		
		// --- RENDER ----
		if (render) {
			render_counter++;
			// STAT: 23ms
			oled_cmd(0x5c);
			oled_data_burst_start();
			int16_t row ;
			
			if (shutdown) {
				
				// --- Y coord. ----
				if (shutdown_offset > 0) {
					for (row=127; row>=65+shutdown_offset; row--)
					{
						for ( pix=0; pix!=160;pix++) {
							oled_data_burst_wr(0);
						}
					}
					
					for (row = 64 + shutdown_offset ; row>=64 - shutdown_offset; row--) {
						for ( pix=0; pix!=160;pix++) {
							pixel = (uint8_t)image[((uint16_t)80*(row>>1))+(pix>>1)] ;
							color = flir_palette[pixel] ;
							oled_data_burst_wr(color) ;
						}
					}
					
					for (row=63 - shutdown_offset; row>=0; row--)
					{
						for ( pix=0; pix!=160;pix++) {
							oled_data_burst_wr(0);
						}
					}
				}
				// --- X coord. ---
				else {
					for (row=127; row>=65; row--)
					{
						for ( pix=0; pix!=160;pix++) {
							oled_data_burst_wr(0);
						}
					}
					
					for ( pix=0; pix!=160;pix++) {
						pixel = (uint8_t)image[((uint16_t)80*(64>>1))+(pix>>1)] ;
						color = pix > 81 + shutdown_offset_x || pix < 79 - shutdown_offset_x ? 0 : flir_palette[pixel] ;
						oled_data_burst_wr(color) ;
					}
					
					for (row=63 ; row>=0; row--)
					{
						for ( pix=0; pix!=160;pix++) {
							oled_data_burst_wr(0);
						}
					}
				}
				
				// --- ANIM ---
				if (shutdown_offset > 0) {
					shutdown_offset-=5	;
					if (shutdown_offset < 0) shutdown_offset = 0;
				}
				
				if (shutdown_offset == 0 && shutdown_offset_x>0) {
					shutdown_offset_x-=5;
					if (shutdown_offset_x < 0) shutdown_offset_x = 0;
				}
				if (shutdown_offset == 0 && shutdown_offset_x == 0) {
					actions(ACTION_SHUT_DOWN) ;
				}
				
			}
			else {
				if (ROI_value>0) {
					image[80*ROI_settings[ROI_value].y_start + ROI_settings[ROI_value].x_start] = 0xff ;
					image[80*ROI_settings[ROI_value].y_start + ROI_settings[ROI_value].x_start+1] = 0xff ;
					image[80*(ROI_settings[ROI_value].y_start+1) + ROI_settings[ROI_value].x_start] = 0xff ;
					
					image[80*ROI_settings[ROI_value].y_start + ROI_settings[ROI_value].x_end] = 0xff ;
					image[80*ROI_settings[ROI_value].y_start + ROI_settings[ROI_value].x_end-1] = 0xff ;
					image[80*(ROI_settings[ROI_value].y_start+1) + ROI_settings[ROI_value].x_end] = 0xff ;
					
					image[80*ROI_settings[ROI_value].y_end + ROI_settings[ROI_value].x_start] = 0xff ;
					image[80*ROI_settings[ROI_value].y_end + ROI_settings[ROI_value].x_start+1] = 0xff ;
					image[80*(ROI_settings[ROI_value].y_end-1) + ROI_settings[ROI_value].x_start] = 0xff ;
					
					image[80*ROI_settings[ROI_value].y_end + ROI_settings[ROI_value].x_end] = 0xff ;
					image[80*ROI_settings[ROI_value].y_end + ROI_settings[ROI_value].x_end-1] = 0xff ;
					image[80*(ROI_settings[ROI_value].y_end-1) + ROI_settings[ROI_value].x_end] = 0xff ;
				}
				
				for (row = 7; row>=0; row--) {
					for ( pix=0; pix!=160;pix++) {
						color = osd[row*160+pix];
						oled_data_burst_wr(color) ;
					}
				}
				for (row = 119; row>=0; row--) {
					for ( pix=0; pix!=160;pix++) {
						pixel = (uint8_t)image[((uint16_t)80*(row>>1))+(pix>>1)] ;
						color = flir_palette[pixel] ;
						oled_data_burst_wr(color) ;
					}
				}
				
				render = false;
			}
			
			oled_data_burst_end();
		}
		
		if (!shutdown && !sleepmode && app_mode == MODE_CAMERA ) {
			
			// STAT: CS_LOW: 150us | CS_HIGH:400us  @ 12MHz
			// CS_LOW: 330us @ 4MHz
			spi_select_slave(&spi_master_instance, &slave_inst, true);
			dma_start_transfer_job(&dma_resource_rx);
			dma_start_transfer_job(&dma_resource_tx);
			while (!transfer_rx_is_done);							// esetleg itt IDLE?
			transfer_rx_is_done = false;
			spi_select_slave(&spi_master_instance, &slave_inst, false);
			
			
			spi_reads++;
			if (getFrame) {
				if ((data[0] & 0x0f) != 0x0f) {
					line_id++;
					//lines[line_id] = data[1];
					if (data[1] > 62) {
						resync = true;
					}
					if (line_id != data[1]) {
						spi_error = line_id;
					}
					if (line_id>2) {
						for (uint16_t p=0; p!=80; p++) {
							uint16_t value = ((data[4+p*2]<<8) + data[4+p*2+1]) ;
							image[(uint16_t)80*(line_id-3)+p]= value;
							if (value < min_val ) min_val = value;
							if (value > max_val ) max_val = value ;
						}
					}
					if (line_id == 62) {
						render = true;
						getFrame = false;
						
						//avg_value = (image[39+29*80] + image[40+29*80] + image[39+30*80] + image[40+30*80]) >> 2 ;
						
						// AGC
						if (sw_agc) {
							uint16_t dinamic_range = (max_val - min_val);
							if (dinamic_range < 255) dinamic_range = 255;
							uint32_t gain = (0xff << 16) / dinamic_range;
							
							image_p = image;
							for (uint16_t pix=0; pix!=60*80; pix++) {
								*image_p = ((*image_p-min_val) * gain) >> 16 ;
								//*image_p = ((*image_p-min_val) ) >> 6 ;
								image_p++;
							}
						}
						
						
					}
				}
			}
			
			if ( (data[0] & 0x0f) != 0x0f && data[1] == 0x00 ) {  // TELEMERY VERSION 9
				frame_id_1 = frame_id;
				frame_id = *(uint32_t*)(data+4+40) ; //(uint32_t)data[44]<<24 | (uint32_t)data[45]<<16 | (uint32_t)data[46]<<8 | (uint32_t)data[47];
				//frames[frame_p] = frame_id; // debug
				//frame_p = (frame_p+1)&0x0f; // debug
				
				
				//strings[0].chars[0] = 0;
				//if (*(uint16_t*)(data+4+3*2+1) & 0x08) { // FFC ikon
				//strings[0].chars[0] = 0x7f ;
				//strings[0].chars[1] = 0x00 ;
				//} // MSB,LSB | LW , HW
				//sprintf(strings[1].chars,"%d", (swap16(*(int16_t*)(data+4+24*2)) - 27315)/100 );
				//sprintf(strings[2].chars,"%d", (swap16(*(int16_t*)(data+4+26*2)) - 27315)/100 );
				//sprintf(strings[3].chars,"%d", avg_value );
				
				strings[0].color = COLOR_BLACK;
				if (*(uint16_t*)(data+4+3*2+1) & 0x08) { // FFC ikon
					strings[0].color =  COLOR_RED;
				}
				if (*(uint16_t*)(data+4+3*2+1) & 0b00110000 == 0b00100000) { // FFC in progress
					strings[0].color =  COLOR_BLUE;
				}
				
				render_osd();
				
				if ( frame_id != frame_id_1 ) { //
					getFrame = true;
					line_id = 0;
					spi_reads = 0;
					min_val= 0x3fff;
					max_val=0 ;
					
					
				}
			}
			if ( spi_reads> 4*3*60 || resync ) {
				delay_ms(200) ;
				spi_reads = 0;
				resync = false;
				render = true;
			}
		}
	}
	
	
	
	
	while(1);
}

#define WIDTH  80
#define HEIGHT 60

void replay_img( char *fname) {
	uint8_t errors = 0;
	
	Ctrl_status status;
	FRESULT res;
	FATFS fs;
	FIL file_object;
	
	oled_cmd(0x15); //Set Column Address
	oled_data_wr(0x00) ;
	oled_data_wr(0x9f) ;
	
	oled_cmd(0x75); //Set Row Address
	oled_data_wr(0x08) ;
	oled_data_wr(0x7f) ;
	
	sd_mmc_init();
	uint8_t retry = 5;
	do {
		status = sd_mmc_test_unit_ready(0);
		res = status ;		// SD READY
		
		retry--;
		delay_ms(5) ;
	} while (CTRL_GOOD != status && retry > 0);
	if (status != CTRL_GOOD) return ;  // NO CARD --> EXIT
	
	memset(&fs, 0, sizeof(FATFS));
	res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
	
	if (FR_INVALID_DRIVE != res) {
		pic_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
		res = f_open(&file_object, 	(char const *)fname, FA_READ );
		
		oled_cmd(0x5c);
		oled_data_burst_start();
		
		if (res == FR_OK) {
			f_lseek(&file_object,54);
			
			for ( int16_t y=HEIGHT-1; y>=0; y-- )
			{
				uint8_t data[3*WIDTH];
				uint16_t result ;
				f_read(&file_object,data,3*WIDTH,result) ;
				for ( uint16_t x=0; x<WIDTH; x++ )
				{
					uint16_t val = color565(data[x*3+2],data[x*3+1],data[x*3+0])  ;
					oled_data_burst_wr(val) ;
					oled_data_burst_wr(val) ;
				}
				for ( uint16_t x=0; x<WIDTH; x++ )
				{
					uint16_t val = color565(data[x*3+2],data[x*3+1],data[x*3+0])  ;
					oled_data_burst_wr(val) ;
					oled_data_burst_wr(val) ;
				}
			}
		}
		else {
			errors++;
		}
	}
	else {
		errors++;
	}
	oled_data_burst_end();
	
	res = f_close(&file_object);
	
	oled_cmd(0x15); //Set Column Address
	oled_data_wr(0x00) ;
	oled_data_wr(0x9f) ;
	
	oled_cmd(0x75); //Set Row Address
	oled_data_wr(0x00) ;
	oled_data_wr(0x7f) ;
}

void save_img_to_sd(void) {
	uint8_t errors = 0;
	
	struct rtc_calendar_time time;
	
	//FRESULT log[16] ;
	
	//char pic_file_name[20] ; //= "0:test-2.txt";
	
	Ctrl_status status;
	FRESULT res;
	FATFS fs;
	FIL file_object;
	
	sd_mmc_init();
	//get_buttons(&buttons_state);
	//if (buttons_state.button_up == false) {
	rtc_calendar_get_time(&rtc_instance, &time) ;
	//sprintf(pic_file_name,"0:test-%d_%d_%d.txt",time.hour,time.minute,time.second);
	sprintf(pic_file_name,"0:image-%d_%d_%d.bmp",time.hour,time.minute,time.second);
	uint8_t retry = 5;
	do {
		status = sd_mmc_test_unit_ready(0);
		res = status ;		// SD READY
		
		//if (CTRL_FAIL == status) {
		//
		//nop_cmd();
		//
		//while (CTRL_NO_PRESENT != sd_mmc_check(0)) {
		//}
		//}
		retry--;
		delay_ms(5) ;
	} while (CTRL_GOOD != status && retry > 0);
	if (status != CTRL_GOOD) return ;  // NO CARD --> EXIT
	
	memset(&fs, 0, sizeof(FATFS));
	res = f_mount(LUN_ID_SD_MMC_0_MEM, &fs);
	//log[1] = res;  // SD MOUNT FS
	
	
	
	if (FR_INVALID_DRIVE != res) {
		pic_file_name[0] = LUN_ID_SD_MMC_0_MEM + '0';
		res = f_open(&file_object, 	(char const *)pic_file_name, FA_CREATE_ALWAYS | FA_WRITE);
		//log[2] = res;   // FILE OPEN
		
		if (res == FR_OK) {
			//res = f_puts("Hello World\n", &file_object) ;
			
			//uint16_t test_palette[4] = {0, 0b1111100000000000 , 0b0000011111100000, 0b0000000000011111 } ;
			//test_bmp(image,test_palette);  // test lines
			//test_bmp(image,color_palette);
			//res = write_bmp(image,test_palette,&file_object) ;
			res = write_bmp(image,flir_palette,&file_object) ;
			
			//log[3] = res;   // FILE WRITE
		}
		else {
			errors++;
		}
	}
	else {
		errors++;
	}
	
	res = f_close(&file_object);
	//log[4] = res;   // FILE CLOSE
	//}
	//system_sleep();
	
}

void actions(uint8_t action) {
	struct extint_chan_conf config_extint_chan;
	
	switch (action) {
		case ACTION_FFC: {
			// FCC command
			LEP_I2C_RunCommand(FLR_CID_SYS_RUN_FFC);
			selected_button = 0;
		}
		break;
		
		case ACTION_SAVE: {
			// SAVE TO DISK
			save_img_to_sd();
		}
		break;
		
		case ACTION_START_SHUT_DOWN: {
			shutdown = true;
			shutdown_offset = 63;
			shutdown_offset_x = 79;
			render = true;
			
			selected_button = 0;
		}
		break;
		
		case ACTION_SHUT_DOWN: {
			// turn off
			port_pin_set_output_level(FLIR_PWR_DWN,0);
			delay_ms(100);
			port_pin_set_output_level(FLIR_CLK_EN,0) ;
			port_pin_set_output_level(V18_ON, 0);
			delay_ms(100);
			port_pin_set_output_level(POWER_ON, 0);
		}
		break;
		
		case ACTION_SLEEP: {
			selected_button = 0;
			
			// powerdown pheripherials
			render = false;
			port_pin_set_output_level(FLIR_PWR_DWN,0);
			delay_ms(100);
			port_pin_set_output_level(FLIR_CLK_EN,0) ;
			port_pin_set_output_level(V18_ON, 0);
			delay_ms(100);
			port_pin_set_output_level(V3_7_ON,0) ;
			sleepmode = true ;
			
			for (uint16_t p=0; p!=4800; p++) {
				image[p] = 5;
			}
			
			
			extint_chan_get_config_defaults(&config_extint_chan);
			config_extint_chan.gpio_pin           = BTN_UP;
			config_extint_chan.gpio_pin_mux       = MUX_PB00A_EIC_EXTINT0   ;
			config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
			config_extint_chan.detection_criteria = EXTINT_DETECT_LOW ;
			config_extint_chan.wake_if_sleeping = true;
			config_extint_chan.filter_input_signal = false;
			extint_chan_set_config( 0  , &config_extint_chan);
			
			adc_disable(&adc_instance);
			spi_disable(&spi_master_instance);
			i2c_master_disable(&i2c_master_instance);
		}
		break;
		
		case ACTION_WAKE_UP: {
			adc_enable(&adc_instance);
			spi_enable(&spi_master_instance);
			i2c_master_enable(&i2c_master_instance);
			
			extint_chan_get_config_defaults(&config_extint_chan);
			config_extint_chan.gpio_pin           = BTN_UP;
			config_extint_chan.gpio_pin_mux       = MUX_PB00A_EIC_EXTINT0   ;
			config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
			config_extint_chan.detection_criteria = EXTINT_DETECT_NONE ;
			config_extint_chan.wake_if_sleeping = true;
			config_extint_chan.filter_input_signal = false;
			extint_chan_set_config( 0  , &config_extint_chan);
			
			port_pin_set_output_level(V3_7_ON,1) ;
			
			oled_init() ;
			
			// POWER ON FLIR
			
			port_pin_set_output_level(FLIR_PWR_DWN, 1);
			port_pin_set_output_level(FLIR_RST, 0);
			port_pin_set_output_level(FLIR_CLK_EN, 1);
			
			delay_ms(250);
			
			port_pin_set_output_level(FLIR_RST, 1);
			
			delay_ms(1000);
			
			flir_setup();
			render = true ;
		}
		break;
		
		case ACTION_PALETTE: {
			flir_palette = flir_palette == color_palette ? green_palette : color_palette ;
		}
		break;
		
		case ACTION_PACMAN: {
			selected_button = 0;
			
			port_pin_set_output_level(FLIR_PWR_DWN,0);
			delay_ms(100);
			port_pin_set_output_level(FLIR_CLK_EN,0) ;
			
			pacman(pac_image) ;
			
			initStrings(false);
			
			port_pin_set_output_level(FLIR_PWR_DWN, 1);
			port_pin_set_output_level(FLIR_RST, 0);
			port_pin_set_output_level(FLIR_CLK_EN, 1);
			
			delay_ms(250);
			
			port_pin_set_output_level(FLIR_RST, 1);
			
			delay_ms(1000);
			
			flir_setup();
		}
		break;
		
		case ACTION_BRIGHTNESS: {
			oled_brightness = (oled_brightness+1) & 0x03;
			oled_cmd(0x87); //Master current control
			oled_data_wr(brightness_levels[oled_brightness].master);
			oled_cmd(0x81); //Set contrast level for R
			oled_data_wr(brightness_levels[oled_brightness].r); //Red contrast set
			oled_cmd(0x82); //Set contrast level for G
			oled_data_wr(brightness_levels[oled_brightness].g); //Green contrast set
			oled_cmd(0x83); //Set contrast level for B
			oled_data_wr(brightness_levels[oled_brightness].b); //Blue contrast set
		}
		break;
		
		case ACTION_ROI: {
			ROI_value = (ROI_value+1) & 0x03 ;
			volatile uint16_t result = LEP_I2C_SetAttribute( ( uint16_t)LEP_CID_AGC_ROI, ( uint16_t* ) &ROI_settings[ROI_value], 4 );
		}
		break;
		
		case ACTION_REPLAY: {
			app_mode = MODE_REPLAY;
			
			replay_img(pic_file_name) ;
			do
			{
				get_buttons(&buttons_state);
			} while (!(buttons_state.button_down != buttons_state.button_down_1 && buttons_state.button_down == false));
			
			app_mode = MODE_CAMERA;
		}
		break;
		
		case ACTION_AGC: {
			uint16_t result;
			sw_agc = !sw_agc ;
			if (sw_agc) {
				uint32_t agcEnableState = (uint32_t)0x00000000 ;  // DISABLE AGC
				result = LEP_I2C_SetAttribute( ( uint16_t )LEP_CID_AGC_ENABLE_STATE, ( uint16_t* ) &agcEnableState, 2 );
			}
			else {
				uint32_t agcEnableState = (uint32_t)0x00000001 ;
				result = LEP_I2C_SetAttribute( ( uint16_t )LEP_CID_AGC_ENABLE_STATE, ( uint16_t* ) &agcEnableState, 2 );
			}
			strings[10].chars[0] = sw_agc ? 0x8f : 0x90;
		}
		break;
		
		case ACTION_CHART: {
			app_mode = MODE_CHART;
			for (uint16_t p=0; p!=4800; p++) {
				image[p] = 0;
			}
			for (uint8_t x=0;x!=80;x++) {
				image[x+43*80] = 10 ;
				image[x+21*80] = 10 ;
				image[x&0b11111000] = 10;
				image[(x&0b11111000)+80] = 10;
				image[(x&0b11111000)+2*80] = 10;
				image[x&0b11111000+59*80] = 10;
				image[(x&0b11111000)+58*80] = 10;
				image[(x&0b11111000)+57*80] = 10;
			}
			for (uint8_t i=0; i!=80; i++){
				image[i+ 80 * (59 - adc_result_buffer[i] ) ] = 0xff;
			}
			render = true;
		}
		break;
	}
}

//  ---=== TEST ===---

void port_test(void) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction  = PORT_PIN_DIR_OUTPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config( FLIR_CS, &config_port_pin);
	port_pin_set_config( FLIR_MISO, &config_port_pin);
	port_pin_set_config( FLIR_SCLK, &config_port_pin);
	port_pin_set_config( FLIR_MOSI, &config_port_pin);
	port_pin_set_config( FLIR_SCL, &config_port_pin);
	port_pin_set_config( FLIR_SDA, &config_port_pin);
	
	//port_pin_set_config( SD_CS, &config_port_pin);
	//port_pin_set_config( SD_MISO, &config_port_pin);
	//port_pin_set_config( SD_SCLK, &config_port_pin);
	//port_pin_set_config( SD_MOSI, &config_port_pin);
	//port_pin_set_config( SD_CARD_DETECT, &config_port_pin);		// IN
	
	port_pin_set_config( OLED_FR, &config_port_pin);			// IN
	
	
	//port_pin_set_config( BTN_UP, &config_port_pin);			// IN
	//port_pin_set_config( BTN_R, &config_port_pin);		// IN
	//port_pin_set_config( BTN_L, &config_port_pin);			// IN
	//port_pin_set_config( BTN_DWN, &config_port_pin);		// IN
	
	while (1)
	{
		uint16_t data = 0x0000 ;
		
		//  stage 1 - DATA LINES TEST
		
		for (uint16_t i = 0; i!=16; i++) {
			*(uint16_t*)(((uint8_t*)PORT_IOBUS)+0x10) = data;
			data += 0x1111 ;
		}
		
		port_pin_set_output_level(OLED_WR, 0);
		port_pin_set_output_level(OLED_RD, 1);
		port_pin_set_output_level(OLED_RST, 0);
		port_pin_set_output_level(OLED_CS, 1);
		port_pin_set_output_level(OLED_DC, 0);
		
		port_pin_set_output_level(FLIR_CS, 0);
		port_pin_set_output_level(FLIR_MISO, 1);
		port_pin_set_output_level(FLIR_SCLK, 0);
		port_pin_set_output_level(FLIR_MOSI, 1);
		port_pin_set_output_level(FLIR_SCL, 0);
		port_pin_set_output_level(FLIR_SDA, 1);
		port_pin_set_output_level(FLIR_RST, 0);
		port_pin_set_output_level(FLIR_PWR_DWN, 1);
		port_pin_set_output_level(OLED_FR, 0);
		
		//port_pin_set_output_level(FLIR_CLK_EN, 0);
		//port_pin_set_output_level(SD_CS, 0);
		//port_pin_set_output_level(SD_MISO, 1);
		//port_pin_set_output_level(SD_SCLK, 0);
		//port_pin_set_output_level(SD_MOSI, 1);
		
		//port_pin_set_output_level(SD_CARD_DETECT, 1);
		//port_pin_set_output_level(BTN_DWN, 0);
		//port_pin_set_output_level(BTN_L, 1);
		//port_pin_set_output_level(BTN_R, 0);
		//port_pin_set_output_level(BTN_UP, 1);
		
		nop_cmd();
		port_pin_set_output_level(OLED_WR, 1);
		port_pin_set_output_level(OLED_RD, 0);
		port_pin_set_output_level(OLED_RST, 1);
		port_pin_set_output_level(OLED_CS, 0);
		port_pin_set_output_level(OLED_DC, 1);
		
		port_pin_set_output_level(FLIR_CS, 1);
		port_pin_set_output_level(FLIR_MISO, 0);
		port_pin_set_output_level(FLIR_SCLK, 1);
		port_pin_set_output_level(FLIR_MOSI, 0);
		port_pin_set_output_level(FLIR_SCL, 1);
		port_pin_set_output_level(FLIR_SDA, 0);
		
		port_pin_set_output_level(FLIR_RST, 1);
		port_pin_set_output_level(FLIR_PWR_DWN, 0);
		
		port_pin_set_output_level(OLED_FR, 1);
		//port_pin_set_output_level(FLIR_CLK_EN, 1);
		
		//port_pin_set_output_level(SD_CS, 1);
		//port_pin_set_output_level(SD_MISO, 0);
		//port_pin_set_output_level(SD_SCLK, 1);
		//port_pin_set_output_level(SD_MOSI, 0);
		
		//port_pin_set_output_level(SD_CARD_DETECT, 0);
		//
		//port_pin_set_output_level(BTN_DWN, 1);
		//port_pin_set_output_level(BTN_L, 0);
		//port_pin_set_output_level(BTN_R, 1);
		//port_pin_set_output_level(BTN_UP, 0);
		
		
	}
	
}

void oled_test(void) {
	//  stage 2 - COLORS TEST
	
	//while (1)
	{
		uint16_t data = 0x0000 ;
		
		data = 0x003f;
		oled_cmd(0x5c);
		for(uint16_t i=0;i<20480;i++)
		{
			oled_data_wr(data);
		}
		data = data << 6 ;
		delay_ms(500);
		
		oled_cmd(0x5c);
		for(uint16_t i=0;i<20480;i++)
		{
			oled_data_wr(data);
		}
		data = data << 6 ;
		delay_ms(500);
		
		oled_cmd(0x5c);
		for(uint16_t i=0;i<20480;i++)
		{
			oled_data_wr(data);
		}
		delay_ms(500);
	}
	
	//  stage 3 - DEMO
	demo();
}

