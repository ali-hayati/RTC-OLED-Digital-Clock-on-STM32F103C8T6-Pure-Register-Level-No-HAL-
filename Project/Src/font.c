/*
 * font.c
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */


#include "font.h"

/* Each row has 5 bytes (columns 0..4); bit r of each byte = pixel of row r
* (0=high .. 6=low). Hand drawn based on standard 5x7 dot grid. */
static const uint8_t FONT_SPACE[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t FONT_MINUS[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
static const uint8_t FONT_SLASH[5] = {0x20, 0x10, 0x08, 0x04, 0x02};
static const uint8_t FONT_COLON[5] = {0x00, 0x00, 0x36, 0x00, 0x00};

static const uint8_t FONT_0[5] = {0x3E, 0x41, 0x41, 0x41, 0x3E};
static const uint8_t FONT_1[5] = {0x00, 0x42, 0x7F, 0x40, 0x00};
static const uint8_t FONT_2[5] = {0x42, 0x61, 0x51, 0x49, 0x46};
static const uint8_t FONT_3[5] = {0x22, 0x41, 0x49, 0x49, 0x36};
static const uint8_t FONT_4[5] = {0x18, 0x14, 0x12, 0x7F, 0x10};
static const uint8_t FONT_5[5] = {0x27, 0x45, 0x45, 0x45, 0x39};
static const uint8_t FONT_6[5] = {0x3C, 0x4A, 0x49, 0x49, 0x30};
static const uint8_t FONT_7[5] = {0x01, 0x71, 0x09, 0x05, 0x03};
static const uint8_t FONT_8[5] = {0x36, 0x49, 0x49, 0x49, 0x36};
static const uint8_t FONT_9[5] = {0x06, 0x49, 0x49, 0x29, 0x1E};

const uint8_t *font5x7_get(char c)
{
    switch (c) {
        case '0': return FONT_0;
        case '1': return FONT_1;
        case '2': return FONT_2;
        case '3': return FONT_3;
        case '4': return FONT_4;
        case '5': return FONT_5;
        case '6': return FONT_6;
        case '7': return FONT_7;
        case '8': return FONT_8;
        case '9': return FONT_9;
        case ':': return FONT_COLON;
        case '-': return FONT_MINUS;
        case '/': return FONT_SLASH;
        case ' ': /* fallthrough */
        default:  return FONT_SPACE;/* Unsupported character -> space, not crash */
    }
}
