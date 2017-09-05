#include <asf.h>

#include "init.h"
#include "oled_ssd1353.h"

extern volatile bool adc_read_done ; 

void init_board (void) {

	
	// gpio

	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	
	// BUTTONS
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(BTN_UP, &config_port_pin);
	port_pin_set_config(BTN_DWN, &config_port_pin);
	port_pin_set_config(BTN_L, &config_port_pin);
	port_pin_set_config(BTN_R, &config_port_pin);
	
	// POWER
	config_port_pin.direction  = PORT_PIN_DIR_OUTPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(POWER_ON, &config_port_pin);
	port_pin_set_config(V18_ON, &config_port_pin);
	port_pin_set_config(V3_7_ON, &config_port_pin);
	port_pin_set_config(V2_8V_OFF, &config_port_pin);
	port_pin_set_config(BATT_DIV_ON, &config_port_pin);
	
	port_pin_set_output_level(POWER_ON, 1);
	port_pin_set_output_level(V18_ON, 0);
	port_pin_set_output_level(V3_7_ON, 1);
	port_pin_set_output_level(V2_8V_OFF, 0);
	port_pin_set_output_level(BATT_DIV_ON, 0);
	
	//config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	//config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	//port_pin_set_config(VBATT_2_IN, &config_port_pin);
	
	// FLIR
	config_port_pin.direction  = PORT_PIN_DIR_OUTPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(FLIR_RST, &config_port_pin);
	port_pin_set_config(FLIR_PWR_DWN, &config_port_pin);
	port_pin_set_config(FLIR_CLK_EN, &config_port_pin);
	
	port_pin_set_output_level(FLIR_PWR_DWN, 0);
	port_pin_set_output_level(FLIR_RST, 1);
	port_pin_set_output_level(FLIR_CLK_EN, 0);
		
	// OLED

	config_port_pin.direction  = PORT_PIN_DIR_OUTPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config( OLED_WR, &config_port_pin);
	port_pin_set_config( OLED_RD, &config_port_pin);
	port_pin_set_config( OLED_RST, &config_port_pin);
	port_pin_set_config( OLED_CS, &config_port_pin);
	port_pin_set_config( OLED_DC, &config_port_pin);
	
	port_pin_set_output_level(OLED_WR, 1);
	port_pin_set_output_level(OLED_RD, 1);
	port_pin_set_output_level(OLED_RST, 1);
	port_pin_set_output_level(OLED_CS, 1);
	port_pin_set_output_level(OLED_DC, 0);
	
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_NONE;
	port_pin_set_config(OLED_FR, &config_port_pin);
	
	// SD CARD
	config_port_pin.direction  = PORT_PIN_DIR_INPUT;
	config_port_pin.input_pull = PORT_PIN_PULL_UP;
	port_pin_set_config(SD_CARD_DETECT, &config_port_pin);
	
	
		
	// DATA LINES
	
	*(uint16_t*)(((uint8_t*)PORT)+0x00) = 0xffff;  // 16 out  -->  INIT_board
	*(uint16_t*)(((uint8_t*)PORT)+0x24) = 0xffff;  // CTRL sampling - 1 // continuous sampling
	
	for (uint8_t i=0; i!=16; i++ ) {
		*((uint8_t*)PORT+0x40+i ) = 0x02;
	}
		
	delay_ms(10);
	
	// POWER ON FLIR
	
	port_pin_set_output_level(FLIR_PWR_DWN, 1);
	port_pin_set_output_level(FLIR_RST, 0);
	port_pin_set_output_level(FLIR_CLK_EN, 1);   
	
	delay_ms(250);
	
	port_pin_set_output_level(FLIR_RST, 1);
	
	delay_ms(1000);
	
	// ADC   -  AIN-2
	// sample time > 400ns
	
	struct adc_config config_adc;
	adc_get_config_defaults(&config_adc);
	
	config_adc.accumulate_samples = ADC_ACCUMULATE_DISABLE ;
	config_adc.clock_prescaler = ADC_CLOCK_PRESCALER_DIV32 ;
	config_adc.clock_source = GCLK_GENERATOR_0;
	//config_adc.differential_mode = false;
	//config_adc.correction = 0 ;
	config_adc.divide_result = ADC_DIVIDE_RESULT_DISABLE ;
	config_adc.gain_factor = ADC_GAIN_FACTOR_DIV2;
	config_adc.positive_input = ADC_POSITIVE_INPUT_PIN2 ;
	config_adc.reference = ADC_REFERENCE_INT1V ;
	config_adc.resolution = ADC_RESOLUTION_10BIT ;
	config_adc.sample_length = 0 ;
	
	adc_init(&adc_instance, ADC, &config_adc);
	adc_enable(&adc_instance);
	
	adc_register_callback(&adc_instance,adc_complete_callback, ADC_CALLBACK_READ_BUFFER );
	adc_enable_callback(&adc_instance, ADC_CALLBACK_READ_BUFFER); 
	
	// --- EXTINT ---
	
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	config_extint_chan.gpio_pin           = BTN_UP;
	config_extint_chan.gpio_pin_mux       = MUX_PB00A_EIC_EXTINT0   ;
	config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_NONE ;
	config_extint_chan.wake_if_sleeping = true;
	config_extint_chan.filter_input_signal = false;
	extint_chan_set_config( 0  , &config_extint_chan);
	
	extint_register_callback(extint_detection_callback, 0,  EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(0, EXTINT_CALLBACK_TYPE_DETECT);
	
	// --- SLEEP ---
		
	//system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
}

extern void actions(uint8_t) ;
extern bool sleepmode; 
extern bool wake_me_up;

void extint_detection_callback(void)
{
	nop_cmd() ;
	if (sleepmode == true) {
		wake_me_up = true;
	}
}

void adc_complete_callback(struct adc_module *const module)
{
	adc_read_done = true;
}

extern uint16_t adc_result; 
extern void startADC(void) ;
extern uint8_t adc_pointer;

void rtc_match_callback(void)
{
	alarm.mask = RTC_CALENDAR_ALARM_MASK_MIN;
	//alarm.time.second += 10;
	//alarm.time.second = alarm.time.second % 60;
	
	rtc_calendar_set_alarm(&rtc_instance, &alarm, RTC_CALENDAR_ALARM_0);
	adc_pointer++;
	if (adc_pointer==80) adc_pointer = 0;
	
	startADC();
}



void rtc_calendar_setup(void)
{
	/* Initialize RTC in calendar mode. */
	struct rtc_calendar_config config_rtc_calendar;
	
	//struct rtc_calendar_time time;
	//
	//rtc_calendar_get_time_defaults(&time);
	//time.year   = 2017;
	//time.month  = 5;
	//time.day    = 16;
	//time.hour   = 10;
	//time.minute = 14;
	//time.second = 24;
	
	rtc_calendar_get_config_defaults(&config_rtc_calendar);
	alarm.time.year      = 2017;
	alarm.time.month     = 5;
	alarm.time.day       = 16;
	alarm.time.hour      = 10;
	alarm.time.minute    = 0;
	alarm.time.second    = 0;
	config_rtc_calendar.continuously_update = true ;
	config_rtc_calendar.clock_24h = true;
	config_rtc_calendar.alarm[0].time = alarm.time;
	config_rtc_calendar.alarm[0].mask = RTC_CALENDAR_ALARM_MASK_MIN ;
	rtc_calendar_init(&rtc_instance, RTC, &config_rtc_calendar);
	rtc_calendar_enable(&rtc_instance);
		
	rtc_calendar_register_callback(
	&rtc_instance, rtc_match_callback, RTC_CALENDAR_CALLBACK_ALARM_0);
	rtc_calendar_enable_callback(&rtc_instance, RTC_CALENDAR_CALLBACK_ALARM_0);
}