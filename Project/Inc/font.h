/*
 * font.h
 *
 *  Created on: Aug 29, 2026
 *      Author: ali Hayati
 */

#ifndef FONT_H_
#define FONT_H_

#include <stdint.h>

/* Simple, self-designed 5x7 font, only for characters needed to display time/date:
* Space, digits 0-9, colon (:), hyphen (-), and slash (/).
*
* Data format: Each character is 5 bytes (one column per byte). In each byte,
* bits 0 to 6 are equivalent to rows from top to bottom (bit 7 is not used).
*
* Support range: ASCII 0x20 to 0x3A only (space to colon). Alphabets
* Not implemented in this phase - a natural extension point for later phases
* (e.g. displaying the name of the day of the week).
*
* @return Pointer to a 5-byte array; for unsupported characters,
* an empty bitmap (space) is returned to avoid crashing the program.
*/
const uint8_t *font5x7_get(char c);

#endif /* FONT_H_ */
