
//#include <asf.h>

#ifndef INIT_H_
#define INIT_H_

// OLED DATA PA00-PA15

#define BTN_UP PIN_PB00
#define BTN_DWN PIN_PB01
#define BTN_R	PIN_PB02
#define BTN_L	PIN_PB03

#define POWER_ON PIN_PB04
#define V18_ON PIN_PB05
#define V3_7_ON PIN_PB06
#define V2_8V_OFF PIN_PB07
#define VBATT_2_IN PIN_PB08
#define BATT_DIV_ON PIN_PB09

#define FLIR_RST PIN_PA27
#define FLIR_PWR_DWN PIN_PA28
#define FLIR_CS PIN_PB23
#define FLIR_MISO PIN_PB22
#define FLIR_CLK_EN PIN_PA21
#define FLIR_SCLK PIN_PA23
#define FLIR_MOSI PIN_PA22
#define FLIR_SCL PIN_PA17
#define FLIR_SDA PIN_PA16

#define OLED_WR PIN_PB16
#define OLED_RD PIN_PB17 
#define OLED_RST PIN_PB31
#define OLED_CS PIN_PB30
#define OLED_DC PIN_PB10
#define OLED_FR PIN_PA18

#define SD_CARD_DETECT PIN_PB11
#define SD_CS PIN_PB15
#define SD_MISO PIN_PB14
#define SD_SCLK PIN_PB13
#define SD_MOSI PIN_PB12

#define nop_cmd() asm volatile("nop")

#define ACTION_FFC 1
#define ACTION_SAVE 2
#define ACTION_SLEEP 3
#define ACTION_START_SHUT_DOWN 4
#define ACTION_PALETTE 5
#define ACTION_PACMAN 6
#define ACTION_BRIGHTNESS 7
#define ACTION_ROI 8
#define ACTION_REPLAY 9
#define ACTION_AGC 10
#define ACTION_CHART 11

#define ACTION_SHUT_DOWN 100
#define ACTION_WAKE_UP 101

#define MODE_SETUP_DATETIME 1
#define MODE_CAMERA 2
#define MODE_REPLAY 3
#define MODE_CHART 4

struct rtc_module rtc_instance;
struct rtc_calendar_alarm_time alarm;

struct adc_module adc_instance;

void init_board (void) ;

void extint_detection_callback(void) ;
void adc_complete_callback(struct adc_module *const );

void rtc_match_callback(void);
void rtc_calendar_setup(void);


#endif /* INIT_H_ */