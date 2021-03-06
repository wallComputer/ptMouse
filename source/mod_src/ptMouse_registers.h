/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.
 * ptMouse_registers.h: Registers in trackball microcontroller.
 */

#ifndef PTMOUSE_REGISTERS_H_
#define PTMOUSE_REGISTERS_H_


#define PTMOUSE_I2C_ADDRESS						0x0A
// #define PTMOUSE_I2C_ADDRESS					0x0B

#define PTMOUSE_CHIP_ID_LOW_REG					0xFA
#define PTMOUSE_CHIP_ID_HIGH_REG				0xFB
#define PTMOUSE_CHIP_ID_VALUE					0xBA11

#define PTMOUSE_VERSION_REG						0xFC
#define PTMOUSE_VERSION_VALUE					0x01

#define PTMOUSE_RED_LED_REGISTER				0x00
#define PTMOUSE_GREEN_LED_REGISTER				0x01
#define PTMOUSE_BLUE_LED_REGISTER				0x02
#define PTMOUSE_WHITE_LED_REGISTER				0x03

#define PTMOUSE_LEFT_MOTION_REG					0x04
#define PTMOUSE_RIGHT_MOTION_REG				0x05
#define PTMOUSE_UP_MOTION_REG					0x06
#define PTMOUSE_DOWN_MOTION_REG					0x07
#define PTMOUSE_SWITCH_REG						0x08
#define PTMOUSE_SWITCH_STATE_MASK				BIT(7)
#define PTMOUSE_SWITCH_PRESSED_STATE			0x01
#define PTMOUSE_SWITCH_RELEASED_STATE			0x00

#define PTMOUSE_INTERRUPT_REG					0xF9
#define PTMOUSE_MASK_INTERRUPT_TRIGGER			BIT(0)
#define PTMOUSE_MASK_INTERRUPT_OUTPUT_ENABLE	BIT(1)

#define PTMOUSE_CONTROL_REG						0xFE
#define PTMOUSE_CONTROL_SLEEP_MASK				BIT(0)
#define PTMOUSE_CONTROL_RESET_MASK				BIT(1)
#define PTMOUSE_CONTROL_FREAD_MASK				BIT(2)
#define PTMOUSE_CONTROL_FWRITE_MASK				BIT(3)


// Unused Registers
#define PTMOUSE_USER_FLASH_REG					0xD0
#define PTMOUSE_FLASH_PAGE_REG					0xF0
#define PTMOUSE_I2C_ADDRESS_REGISTER			0xFD



#endif
