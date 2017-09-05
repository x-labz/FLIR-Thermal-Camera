

#ifndef BUTTONS_H_
#define BUTTONS_H_


typedef struct  {
	bool button_up;
	bool button_down;
	bool button_left;
	bool button_right;
	bool button_up_1;
	bool button_down_1;
	bool button_left_1;
	bool button_right_1;
	uint32_t timestamp;
	}  button_type;
	
	 button_type buttons_state;

void get_buttons( button_type *  ) ;
uint32_t get_time_stamp(void);

#endif /* BUTTONS_H_ */