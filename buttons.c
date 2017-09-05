

#include <asf.h>
#include "buttons.h"
#include "init.h"


void get_buttons( button_type * state ) {
	//struct rtc_calendar_time time;
	//rtc_calendar_get_time(&rtc_instance, &time) ;
	state->timestamp =  SysTick->VAL ;
	
	state->button_up_1 = state->button_up ;
	state->button_down_1 = state->button_down ;
	state->button_left_1 = state->button_left ;
	state->button_right_1 = state->button_right ;	
	
	state->button_up = port_pin_get_input_level(BTN_UP) ;
	state->button_down = port_pin_get_input_level(BTN_DWN) ;
	state->button_left = port_pin_get_input_level(BTN_L) ;
	state->button_right = port_pin_get_input_level(BTN_R) ;	
}

uint32_t get_time_stamp(void) {
	return SysTick->VAL ;
}
