/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.
 * ptMouse_codes.h: Button Layout and Scancodes-Keycodes mapping.
 */
#ifndef PTMOUSE_CODES_H_
#define PTMOUSE_CODES_H_


#include <linux/input.h>

/*
 *	  Pimoroni Trackball Layout
 *  +---------------------------+
 *  |       +-----------+       |
 *  |       |           |       |
 *  |       |  BTN_LEFT |       |
 *  |       |           |       |
 *  |       +-----------+       |
 *  |                           |
 *  +------+             +------+
 *         |             |
 *         |3V3|D|C|I|GND|
 *         |_____________|
 */



/*
 * Orientation 0:
 *		UP
 * LEFT			RIGHT
 *		DOWN
 *
 *
 * Orientation 1:
 *		LEFT
 * DOWN			UP
 *		RIGHT
 *
 *
 * Orientation 2:
 *		DOWN
 * RIGHT		LEFT
 *		UP
 *
 *
 * Orientation 3:
 *		RIGHT
 * UP			DOWN
 *		LEFT
 */


#define PTMOUSE_NUM_KEYCODES	1

static unsigned short ptMouse_keycodes[PTMOUSE_NUM_KEYCODES] = {
	BTN_LEFT,			// CHANGE THIS TO CHANGE BUTTON BEHAVIOUR.
};
#endif
