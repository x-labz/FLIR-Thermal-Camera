
#define ATMEL 

#define break_cmd() asm volatile("BKPT")

#include "libFixMath/fix16.h"
#include "pac-man.h"
#include "oled_ssd1353.h"
#include "pacman_text_bitmaps.h"

const uint16_t pacman_palette[256] = {
	0,
	0b0000000000011111,
	0b0000011111100000,
	0b1111100000000000, // blinky
	0xffff,
	0b0111100000000000,
	0b0000001111100000,
	0b0000000000001111,
	0xFFE0 ,
	0xFDDF , // pinky
	0x07FF, // inky
	0xFDCA // clyde
};

void pacman(uint8_t *image) {
	state_type state;
	state_init(&state);
	const bitmap_type* image_p;
	
	while (1) {
		//   LOOP

		clear_image_pacman(image);
		draw_pacman(image, &state);
		
		get_buttons(&buttons_state);
		
		switch (state.game_state)
		{
			case GAME_GET_READY:
			image_p = &(pacman_texts_array[0]);
			if (!buttons_state.button_up && buttons_state.button_up_1) {
				state.game_state = GAME_RUN;
				image_p = NULL;
			}
			break;
			case GAME_RUN:
			calc_state(&state, &buttons_state);
			break;
			case GAME_GAME_OVER:
			image_p = &pacman_texts_array[1];
			if (!buttons_state.button_up && buttons_state.button_up_1) {
				state_init(&state);
			}
			if (!buttons_state.button_down && buttons_state.button_down_1) {
				return false;
			}
			break;
			case GAME_YOU_WIN:
			image_p = &pacman_texts_array[2];
			if (!buttons_state.button_up && buttons_state.button_up_1) {
				if (!buttons_state.button_up && buttons_state.button_up_1) {
					state_init(&state);
				}
				if (!buttons_state.button_down && buttons_state.button_down_1) {
					return false;
				}
			}
			break;
		}

		if (image_p != NULL && state.text_visible > 8) {
			draw_text((128 - image_p->x) / 2, (128 - image_p->y) / 2, image_p, image);
		}
		state.text_visible++;
		if (state.text_visible > 32) state.text_visible = 0;
		
		
		render_image(image, pacman_palette);

		// END LOOP
	}
}

// ----------

void state_init(state_type* state) {
	state->isGapRotating = 0;
	state->gapRotatingTimer = 1200;
	state->rotatingLevel = 0;

	state->game_state = GAME_GET_READY;
	state->text_visible = 0;

	state->levels[0] = 12;
	state->levels[1] = 32;
	state->levels[2] = 52;

	state->speeds[2] = DEG1;
	state->speeds[1] = DEG1 * 2;
	state->speeds[0] = DEG1 * 4;

	#ifdef ATMEL
	state->gaps[0] = (struct gap_type) { 12, 22, -DEG15, DEG45 };
	state->gaps[1] = (struct gap_type) { 32, 42, DEG60 - DEG6, DEG90 };
	state->gaps[2] = (struct gap_type) { 32, 42, DEG180 + DEG60 - DEG6, DEG180 + DEG90 };
	state->gaps[3] = (struct gap_type) { 52, 63, 0, DEG30 };
	state->gaps[4] = (struct gap_type) { 52, 63, DEG180, DEG180 + DEG30 };
	#else
	state->gaps[0] = { 12, 22,-DEG15 , DEG45 };
	state->gaps[1] = { 32, 42,DEG60 - DEG6 , DEG90 };
	state->gaps[2] = { 32, 42,DEG180 + DEG60 - DEG6, DEG180 + DEG90 };
	state->gaps[3] = { 52, 63,0 , DEG30 };
	state->gaps[4] = { 52, 63,DEG180 , DEG180 + DEG30 };
	#endif

	state->pacman.level = 2;
	state->pacman.r = state->levels[state->pacman.level];
	state->pacman.a = 0;
	state->pacman.anim = 0;
	state->pacman.anim_dir = DEG1;
	state->pacman.dir = DEG1;
	state->pacman.target_r = state->pacman.r;
	state->pacman.state = SPRITE_STANDBY;
	state->pacman.lock = 0;

	state->ghost[0].a = 0;
	state->ghost[0].r = state->levels[0];
	state->ghost[0].move = DEG1;
	state->ghost[0].state = SPRITE_ISCIRCULAR_MOVING;
	state->ghost[0].lock = 20;
	state->ghost[0].level_lock = 0;
	state->ghost[0].level = 0;
	state->ghost[1].a = DEG90;
	state->ghost[1].r = state->levels[0];
	state->ghost[1].move = DEG1;
	state->ghost[1].state = SPRITE_ISCIRCULAR_MOVING;
	state->ghost[1].lock = 20;
	state->ghost[1].level_lock = 0;
	state->ghost[1].level = 0;
	state->ghost[2].a = DEG180;
	state->ghost[2].r = state->levels[0];
	state->ghost[2].move = DEG1;
	state->ghost[2].state = SPRITE_ISCIRCULAR_MOVING;
	state->ghost[2].lock = 20;
	state->ghost[2].level_lock = 0;
	state->ghost[2].level = 0;
	state->ghost[3].a = DEG180 + DEG90;
	state->ghost[3].r = state->levels[0];
	state->ghost[3].move = DEG1;
	state->ghost[3].state = SPRITE_ISCIRCULAR_MOVING;
	state->ghost[3].lock = 20;
	state->ghost[3].level_lock = 0;
	state->ghost[3].level = 0;

	state->ghost[0].color = 3;
	state->ghost[1].color = 9;
	state->ghost[2].color = 10;
	state->ghost[3].color = 11;

	for (int8_t i = 0; i != 42; i++) {
		state->points[i].isEaten = false;
		state->points[i].isHidden = false;
	}

	int16_t point = 0;
	for (fix16_t a = 0; a <= DEG360; a = fix16_add(a, DEG15)) {
		state->points[point].x = fix16_to_int(fix16_mul(fix16_from_int(52), fix16_cos(a))) + 64;
		state->points[point].y = fix16_to_int(fix16_mul(fix16_from_int(52), fix16_sin(a))) + 64;
		state->points[point].r = 52;
		state->points[point].a = a;
		point++;
	}
	for (fix16_t a = 0; a <= DEG360; a = fix16_add(a, DEG30)) {
		state->points[point].x = fix16_to_int(fix16_mul(fix16_from_int(32), fix16_cos(a))) + 64;
		state->points[point].y = fix16_to_int(fix16_mul(fix16_from_int(32), fix16_sin(a))) + 64;
		state->points[point].r = 32;
		state->points[point].a = a;
		point++;
	}
	for (fix16_t a = 0; a <= DEG360; a = fix16_add(a, DEG60)) {
		state->points[point].x = fix16_to_int(fix16_mul(fix16_from_int(12), fix16_cos(a))) + 64;
		state->points[point].y = fix16_to_int(fix16_mul(fix16_from_int(12), fix16_sin(a))) + 64;
		state->points[point].r = 12;
		state->points[point].a = a;
		point++;
	}
	state->points_count = point - 1;
}

bool checkGap(state_type* state, int16_t dir) {
	int16_t level = state->pacman.level;
	bool result = false;
	if (dir == LEVEL_DOWN) {
		level = state->pacman.level - 1;
	}
	if (level >= 0) {
		for (int16_t i = 0; i != 5; i++) {
			if (state->gaps[i].r == state->levels[level] && state->gaps[i].start < fix16_sub(state->pacman.a, DEG12) && state->gaps[i].end > fix16_add(state->pacman.a, DEG12)) {
				result = true;
			}
		}
	}
	return result;
}

bool checkGapGhost(state_type* state, int8_t index, int16_t dir) {
	int16_t level = state->ghost[index].level;
	bool result = false;
	if (dir == LEVEL_DOWN) {
		level = state->ghost[index].level - 1;
	}
	if (level >= 0) {
		if (level > 2 || level <0) break_cmd();
		for (int16_t i = 0; i != 5; i++) {
			if (state->gaps[i].r == state->levels[level] && state->gaps[i].start < fix16_sub(state->ghost[index].a, DEG12) && state->gaps[i].end > fix16_add(state->ghost[index].a, DEG12)) {
				result = true;
				//cout << " s: " << fix16_to_float(state->gaps[i].start) << " e: " << fix16_to_float(state->gaps[i].end) << " g: " << fix16_to_float(state->ghost[index].a) << " i: " << (int16_t)index << " L: " << level << "\n";
			}
		}
	}
	return result;
}

void calc_state(state_type* state, button_type* buttons) {

	// CIRCULAR MOVEMENT
	if (!buttons->button_left && state->pacman.state == SPRITE_STANDBY) {
		state->pacman.dir = -state->speeds[state->pacman.level];  //DEG_1;
		state->pacman.lock = 15;
		state->pacman.state = SPRITE_ISCIRCULAR_MOVING;
	}
	if (!buttons->button_right && state->pacman.state == SPRITE_STANDBY) {
		state->pacman.dir = state->speeds[state->pacman.level];  //DEG1;
		state->pacman.lock = 15;
		state->pacman.state = SPRITE_ISCIRCULAR_MOVING;
	}

	if (state->pacman.state == SPRITE_ISCIRCULAR_MOVING) {
		//if (state->pacman.lock > 0) state->pacman.lock--;
		//if (state->pacman.lock == 0)
		state->pacman.state = SPRITE_STANDBY;
		state->pacman.a = fix16_add(state->pacman.a, state->pacman.dir);
	}

	if (state->pacman.a > DEG360) state->pacman.a = fix16_sub(state->pacman.a, DEG360);
	if (state->pacman.a < 0) state->pacman.a = fix16_add(state->pacman.a, DEG360);

	// LEVEL CHANGE
	if (!buttons->button_up && state->pacman.state == SPRITE_STANDBY && checkGap(state, LEVEL_UP)) {
		state->pacman.level++;
		if (state->pacman.level > 2) {
			state->pacman.level = 2;
			state->pacman.a = fix16_add(state->pacman.a, DEG180);
			state->pacman.lock = 30;
		}
		state->pacman.target_r = state->levels[state->pacman.level];
		state->pacman.state = SPRITE_ISMOVING;
	}

	if (!buttons->button_down && state->pacman.state == SPRITE_STANDBY && checkGap(state, LEVEL_DOWN)) {
		state->pacman.level--;
		if (state->pacman.level < 0) {
			state->pacman.level = 0;
		}
		state->pacman.target_r = state->levels[state->pacman.level];
		state->pacman.state = SPRITE_ISMOVING;
	}

	if (state->pacman.state == SPRITE_ISMOVING) {
		if (state->pacman.lock > 0) state->pacman.lock--;
		if (state->pacman.target_r < state->pacman.r)  state->pacman.r--;
		if (state->pacman.target_r > state->pacman.r) state->pacman.r++;
		if (state->pacman.target_r == state->pacman.r && state->pacman.lock == 0) {
			state->pacman.state = SPRITE_STANDBY;
		}
	}

	// GHOST MOVING
	for (int8_t i = 0; i != 4; i++) {
		if (state->ghost[i].level_lock > 0) state->ghost[i].level_lock--;

		if (state->ghost[i].state == SPRITE_ISCIRCULAR_MOVING) {
			if (state->ghost[i].lock > 0) state->ghost[i].lock--;
			if (state->ghost[i].lock == 0) {
				state->ghost[i].lock = 120;
				if (state->ghost[i].r >= state->levels[1]) {
					state->ghost[i].move = ((get_time_stamp() >> i) & 1) ? DEG1 : DEG_1;
				}
			}
			state->ghost[i].a = fix16_add(state->ghost[i].a, state->ghost[i].move);
			if (state->ghost[i].a > DEG360) state->ghost[i].a = fix16_sub(state->ghost[i].a, DEG360);
			if (state->ghost[i].a < 0) state->ghost[i].a = fix16_add(state->ghost[i].a, DEG360);

			int16_t level = 0;
			bool go_up = checkGapGhost(state, i, LEVEL_UP);
			bool go_down = checkGapGhost(state, i, LEVEL_DOWN);
			if (state->ghost[i].level_lock == 0) {
				if (state->ghost[i].r == state->levels[1] && go_down) {
					go_down = false;
				}

				level = go_up ? LEVEL_UP : go_down ? LEVEL_DOWN : 0;

				level = ((get_time_stamp() >> i) & 1) ? level : 0;
			}

			if (level) {
				if (level == LEVEL_DOWN) {
					state->ghost[i].level--;
					if (state->ghost[i].level < 0) {
						state->ghost[i].level = 0;
					}
					state->ghost[i].target_r = state->levels[state->ghost[i].level];
					state->ghost[i].state = SPRITE_ISMOVING;
				}
				else {
					state->ghost[i].level++;
					if (state->ghost[i].level > 2) {
						state->ghost[i].level = 2;
					}
					state->ghost[i].target_r = state->levels[state->ghost[i].level];
					state->ghost[i].state = SPRITE_ISMOVING;
				}
				state->ghost[i].level_lock = 120;
			}
		}
		if (state->ghost[i].state == SPRITE_ISMOVING) {
			if (state->ghost[i].target_r < state->ghost[i].r)  state->ghost[i].r--;
			if (state->ghost[i].target_r > state->ghost[i].r) state->ghost[i].r++;
			if (state->ghost[i].target_r == state->ghost[i].r) {
				state->ghost[i].state = SPRITE_ISCIRCULAR_MOVING;
			}
		}

		// GAP ROTATING
		state->gapRotatingTimer--;
		if (state->gapRotatingTimer == 0) {
			state->gapRotatingTimer = 1200;
			state->isGapRotating = 60;
			state->rotatingLevel = ((get_time_stamp() | buttons->timestamp) >> 1) & 0x03;
			if (state->rotatingLevel > 2) state->rotatingLevel = 0;
		}
		if (state->isGapRotating > 0) {
			state->isGapRotating--;
			for (int16_t i = 0; i != 5; i++) {
				if (state->gaps[i].r == state->levels[state->rotatingLevel]) {
					state->gaps[i].start = fix16_add(state->gaps[i].start, DEG1);
					state->gaps[i].end = fix16_add(state->gaps[i].end, DEG1);

					if (state->gaps[i].start > DEG360) {
						state->gaps[i].start = fix16_sub(state->gaps[i].start, DEG360);
					}
					if (state->gaps[i].end > DEG360) {
						state->gaps[i].end = fix16_sub(state->gaps[i].end, DEG360);
					}
					if (state->gaps[i].end < state->gaps[i].start) {
						state->gaps[i].end = fix16_add(state->gaps[i].end, DEG360);
					}
				}
			}
		}

	}

	// PACMAN ANIM
	state->pacman.anim = fix16_add(state->pacman.anim, state->pacman.anim_dir);
	if (state->pacman.anim > DEG15) state->pacman.anim_dir = DEG_1;
	if (state->pacman.anim < DEG_3) state->pacman.anim_dir = DEG1;


	// HIDE or EAT POINTS

	for (int8_t i = 0; i != 42; i++) {
		state->points[i].isHidden = false;
		if (!state->points[i].isEaten && state->pacman.r == state->points[i].r &&  state->points[i].a > fix16_sub(state->pacman.a, DEG6) && state->points[i].a < fix16_add(state->pacman.a, DEG6)) {
			state->points[i].isEaten = true;
			state->points_count--;
		}
		for (int8_t g = 0; g != 4; g++) {
			if (state->ghost[g].r == state->points[i].r &&  state->points[i].a > fix16_sub(state->ghost[g].a, DEG12) && state->points[i].a < fix16_add(state->ghost[g].a, DEG12)) {
				state->points[i].isHidden = true;
			}
		}
	}

	// COLLiSION
	bool collision = false;
	for (int8_t g = 0; g != 4; g++) {
		if (state->ghost[g].r == state->pacman.r &&  state->pacman.a > fix16_sub(state->ghost[g].a, DEG15) && state->pacman.a < fix16_add(state->ghost[g].a, DEG15)) {
			collision = true;
			//std::cout << collision;
		}
	}
	if (collision) {
		state->game_state = GAME_GAME_OVER;
	}

	// check win
	if (state->points_count == 0) {
		state->game_state = GAME_YOU_WIN;
	}
}

void draw_pacman(uint8_t *image, state_type* s) {
	fix16_t pac_anim = 0, pac_anim_dir = DEG1;
	coord start_coord, end_coord;

	int16_t current_level = 0;
	fix16_t start_angle = 0;
	for (int8_t i = 0; i != 5; i++) {
		if (current_level != s->gaps[i].r) {
			current_level = s->gaps[i].r;
			start_angle = s->gaps[i].start;
		}
		fix16_t start = s->gaps[i].end;
		fix16_t end = s->gaps[i].r == s->gaps[i + 1].r ? s->gaps[i + 1].start : fix16_add(start_angle, DEG360);
		arc(64, 64, s->gaps[i].draw_r, start, end, image, 1, &start_coord, &end_coord);
	}

	for (int8_t i = 0; i != 42; i++) {
		if (!s->points[i].isHidden && !s->points[i].isEaten) {
			image[WIDTH * s->points[i].y + s->points[i].x] = 4;
		}
	}

	int16_t pac_x = fix16_to_int(fix16_mul(fix16_from_int(s->pacman.r), fix16_cos(s->pacman.a))) + 64;
	int16_t pac_y = fix16_to_int(fix16_mul(fix16_from_int(s->pacman.r), fix16_sin(s->pacman.a))) + 64;

	arc(pac_x,
	pac_y,
	8,
	fix16_add(fix16_add(s->pacman.a, s->pacman.dir > 0 ? DEG105 : DEG105 + DEG180), s->pacman.anim),
	fix16_sub(fix16_add(s->pacman.a, s->pacman.dir > 0 ? DEG435 : DEG435 + DEG180), s->pacman.anim),
	image,
	8,
	&start_coord,
	&end_coord);
	drawline(pac_x, pac_y, start_coord.x, start_coord.y, image, 8);
	drawline(pac_x, pac_y, end_coord.x, end_coord.y, image, 8);

	for (int8_t i = 0; i != 4; i++) {
		int16_t g_x = fix16_to_int(fix16_mul(fix16_from_int(s->ghost[i].r - 2), fix16_cos(s->ghost[i].a))) + 64;
		int16_t g_y = fix16_to_int(fix16_mul(fix16_from_int(s->ghost[i].r - 2), fix16_sin(s->ghost[i].a))) + 64;
		ghost(g_x, g_y, s->ghost[i].a, image, s->ghost[i].color);
	}
}



void clear_image_pacman(uint8_t *image) {
	for (uint16_t p = 0; p != WIDTH*HEIGHT; p++) {
		image[p] = 0;
	}
}

void circle(uint16_t x0, uint16_t y0, uint16_t r, uint8_t *fb, uint8_t color) {
	fix16_t step = DEG1;
	for (fix16_t a = 0; a < DEG360; a = fix16_add(a, step)) {
		int16_t x = fix16_to_int(fix16_mul(fix16_from_int(r), fix16_cos(a)));
		int16_t y = fix16_to_int(fix16_mul(fix16_from_int(r), fix16_sin(a)));

		fb[WIDTH * (y + y0) + x + x0] = color;
	}
}

void ghost(uint16_t x0, uint16_t y0, fix16_t angle, uint8_t *image, uint8_t color) {
	coord start, end;

	angle = fix16_add(angle, DEG90);

	fix16_t points_r[12] = { 262140,419629,	463402,	419629,	463402,	185361,	327675,	236290,	236290,	185361 ,293081,	293081 };
	fix16_t points_a[12] = { -102942,-161665,154413,-44219,51471,	154413,	102942,	167350,	38535,	51471,-72557,	-133327 };

	int16_t points_x[12], points_y[12];

	for (uint16_t i = 0; i != 12; i++) {
		points_x[i] = fix16_to_int(fix16_mul(points_r[i], fix16_cos(fix16_add(angle, points_a[i])))) + x0;
		points_y[i] = fix16_to_int(fix16_mul(points_r[i], fix16_sin(fix16_add(angle, points_a[i])))) + y0;
	}

	arc(points_x[0],
	points_y[0],
	5,
	fix16_add(angle, DEG180),
	fix16_add(angle, DEG360),
	image,
	color,
	&start,
	&end);
	drawline(points_x[1], points_y[1], points_x[2], points_y[2], image, color);
	drawline(points_x[3], points_y[3], points_x[4], points_y[4], image, color);

	drawline(points_x[2], points_y[2], points_x[5], points_y[5], image, color);
	drawline(points_x[6], points_y[6], points_x[7], points_y[7], image, color);
	drawline(points_x[6], points_y[6], points_x[8], points_y[8], image, color);
	drawline(points_x[4], points_y[4], points_x[9], points_y[9], image, color);

	image[WIDTH * points_y[10] + points_x[10]] = color;
	image[WIDTH * points_y[11] + points_x[11]] = color;
}

void arc(uint16_t x0, uint16_t y0, uint16_t r, fix16_t start, fix16_t end, uint8_t *fb, uint8_t color, coord * result_start, coord *result_end) {
	fix16_t step = DEG12;
	fix16_t r_fix = fix16_from_int(r);

	result_start->x = fix16_to_int(fix16_mul(r_fix, fix16_cos(start))) + x0;
	result_start->y = fix16_to_int(fix16_mul(r_fix, fix16_sin(start))) + y0;
	result_end->x = fix16_to_int(fix16_mul(r_fix, fix16_cos(end))) + x0;
	result_end->y = fix16_to_int(fix16_mul(r_fix, fix16_sin(end))) + y0;

	int16_t x_1 = result_start->x;
	int16_t y_1 = result_start->y;

	for (fix16_t a = start; a < end; a = fix16_add(a, step)) {
		int16_t x = fix16_to_int(fix16_mul(r_fix, fix16_cos(a))) + x0;
		int16_t y = fix16_to_int(fix16_mul(r_fix, fix16_sin(a))) + y0;

		drawline(x_1, y_1, x, y, fb, color);
		x_1 = x;
		y_1 = y;
	}
	drawline(x_1, y_1, result_end->x, result_end->y, fb, color);
}

void filled_arc(uint16_t x0, uint16_t y0, uint16_t r, fix16_t start, fix16_t end, uint8_t *fb, uint8_t color) {
	fix16_t step = DEG3;
	fix16_t r_fix = fix16_from_int(r);
	for (fix16_t a = start; a < end; a = fix16_add(a, step)) {
		int16_t x = fix16_to_int(fix16_mul(r_fix, fix16_cos(a))) + x0;
		int16_t y = fix16_to_int(fix16_mul(r_fix, fix16_sin(a))) + y0;

		drawline(x0, y0, x, y, fb, color);
	}
}


void drawline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t *fb, uint8_t color)
{
	int16_t dx, dy, p, x, y;
	x = x1;
	y = y1;
	dx = abs(x2 - x1);
	dy = abs(y2 - y1);

	int16_t s1 = sign(x2 - x1);
	int16_t s2 = sign(y2 - y1);
	int16_t swap = 0;

	if (dy > dx) {
		int16_t temp = dx;
		dx = dy;
		dy = temp;
		swap = 1;
	}
	int16_t D = 2 * dy - dx;

	for (int16_t i = 0; i < dx; i++) {
		fb[WIDTH * y + x] = color;
		while (D >= 0)
		{
			D = D - 2 * dx; if (swap) x += s1; else y += s2;
		}
		D = D + 2 * dy; if (swap) y += s2; else x += s1;
	}
}

void draw_text(uint8_t x0, uint8_t y0, const bitmap_type* bitmap, uint8_t *image) {
	for (int16_t y = 0; y != bitmap->y; y++) {
		for (int16_t x = 0; x != bitmap->x; x++) {
			uint8_t pix = bitmap->data[y*bitmap->x + x];
			if (pix != 0xff) {
				image[(y0 + y)*WIDTH + x0 + x] = pix;
			}
		}
	}
}
// CUSTOM

void render_image(uint8_t *image, const uint16_t *palette) {
	oled_cmd(0x5c);
	oled_data_burst_start();
	
	for (int16_t row = 127; row>=0; row--) {
		for ( uint16_t pix=0; pix!=160;pix++) {
			uint8_t pixel = image[(WIDTH*row)+pix] ;
			uint16_t color = palette[pixel] ;
			oled_data_burst_wr(color) ;
		}
	}
	
	oled_data_burst_end();
}