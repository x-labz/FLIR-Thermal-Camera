

#ifndef PAC-MAN_H_
#define PAC-MAN_H_

#include <asf.h>
#include "libFixMath/fix16.h"
#include "buttons.h"
#include "pacman_text_bitmaps.h"

#define WIDTH 160
#define HEIGHT 128

#define sign(x) ((x > 0)? 1 : ((x < 0)? -1: 0))

#define DEG_3  (fix16_t)0xfffff299
#define DEG_1  (fix16_t)0xfffffb88
#define DEG1   (fix16_t)0x0000047C
#define DEG3   (fix16_t)0x00000D67
#define DEG6   (fix16_t)0x00001ACE
#define DEG12  (fix16_t)0x0000359D
#define DEG15  (fix16_t)0x00004305
#define DEG30  (fix16_t)0x0000860A
#define DEG45  (fix16_t)0x0000C90E
#define DEG60  (fix16_t)0x00010C13
#define DEG90  (fix16_t)0x0001921D
#define DEG105 (fix16_t)0x0001d521
#define DEG180 (fix16_t)0x00032439
#define DEG360 (fix16_t)0x00064873
#define DEG435 (fix16_t)0x0007978B

#define SPRITE_ISCIRCULAR_MOVING 2
#define SPRITE_ISMOVING 1
#define SPRITE_STANDBY 0

#define LEVEL_UP 1
#define LEVEL_DOWN 2

#define GAME_GET_READY 1
#define GAME_RUN 2
#define GAME_GAME_OVER 3
#define GAME_YOU_WIN 4


typedef struct  {
	int16_t x;
	int16_t y;
} coord;

typedef struct {
	struct pac_type {
		int16_t level;
		fix16_t a;
		int16_t r;
		fix16_t anim;
		fix16_t anim_dir;
		fix16_t dir;
		uint8_t state;
		fix16_t target_r;
		int16_t lock;
	} pacman;
	struct ghost_type {
		int16_t level;
		fix16_t a;
		int16_t r;
		uint8_t color;
		fix16_t move;
		uint8_t state;
		fix16_t target_r;
		int16_t lock;
		int16_t level_lock;
	} ghost[4];
	struct point_type {
		int16_t x;
		int16_t y;
		fix16_t a;
		int16_t r;
		bool isEaten;
		bool isHidden;
	} points[44];
	uint8_t points_count;
	struct gap_type {
		int16_t r;
		int16_t draw_r;
		fix16_t start;
		fix16_t end;
	} gaps[5];
	int16_t isGapRotating;
	int16_t gapRotatingTimer;
	uint8_t rotatingLevel;
	int16_t levels[3];
	fix16_t speeds[3];
	uint8_t game_state;
	uint8_t text_visible;
} state_type;


uint32_t get_time_stamp(void);
void circle(uint16_t, uint16_t, uint16_t, uint8_t *, uint8_t);
void drawline(int16_t, int16_t, int16_t, int16_t, uint8_t *, uint8_t);
void arc(uint16_t, uint16_t, uint16_t, fix16_t, fix16_t, uint8_t *, uint8_t, coord *, coord *);
void filled_arc(uint16_t, uint16_t, uint16_t, fix16_t, fix16_t, uint8_t *, uint8_t);
void ghost(uint16_t, uint16_t, fix16_t, uint8_t *, uint8_t);
void render_image(uint8_t *, const uint16_t *);
void state_init(state_type*);
bool checkGap(state_type*, int16_t);
bool checkGapGhost(state_type*, int8_t, int16_t);
void calc_state(state_type*, button_type*);
void draw_pacman(uint8_t *, state_type*);
void clear_image_pacman(uint8_t *);
void draw_text(uint8_t, uint8_t, const bitmap_type*, uint8_t*);

void pacman(uint8_t *);

#endif /* PAC-MAN_H_ */