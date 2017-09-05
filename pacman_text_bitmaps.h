#ifndef PACMAN_TEXT_BITMAPS_H_
#define PACMAN_TEXT_BITMAPS_H_

#include <stdint.h>

typedef struct {
	const uint8_t *data;
	int8_t x;
	int8_t y;
} bitmap_type;

extern const bitmap_type pacman_texts_array[];

#endif