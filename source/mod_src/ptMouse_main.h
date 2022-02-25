/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.
 * ptMouse_main.h: Main H File.
 */
#ifndef PTMOUSE_MAIN_H_
#define PTMOUSE_MAIN_H_

#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/ktime.h>

#include "ptMouse_i2cHelper.h"
#include "ptMouse_registers.h"
#include "ptMouse_codes.h"
#include "debug_levels.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wallComputer");
MODULE_DESCRIPTION("Convert Pimoroni Trackball in to a mouse. Software written by wallComputer");
MODULE_VERSION("0.1");


#define COMPATIBLE_NAME								"wallComputer,ptMouse"
#define DEVICE_NAME									"ptMouse"
#define PTMOUSE_BUS_TYPE							BUS_I2C
#define PTMOUSE_VENDOR_ID							0x0001
#define PTMOUSE_PRODUCT_ID							PTMOUSE_CHIP_ID_VALUE
#define PTMOUSE_VERSION_ID							PTMOUSE_VERSION_VALUE

#define PTMOUSE_DEFAULT_WORK_RATE					40		// Actually sets period to ~20ms. Don't know why?
#define PTMOUSE_MINIMUM_WORK_RATE					10		// Minimum 10ms.
#define PTMOUSE_MAXIMUM_WORK_RATE					1000	// Maximum 1000ms.
#define PTMOUSE_ORIENTATION_0						0		// Looking at the Sensor from above.
#define PTMOUSE_ORIENTATION_90						1		// Rotated 90 degress clockwise.
#define PTMOUSE_ORIENTATION_180						2		// Rotated 180 degress clockwise.
#define PTMOUSE_ORIENTATION_270						3		// Rotated 270 degress clockwise.
#define PTMOUSE_DEFAULT_ORIENTATION					PTMOUSE_ORIENTATION_270		// Defualt to be used with Pimoroni Display Hat Mini and OmnigrammerV4.
#define PTMOUSE_DEFAULT_SCALE_FACTOR				100
#define PTMOUSE_FLIP_ON								1
#define PTMOUSE_FLIP_OFF							0
#define PTMOUSE_DEFAULT_FLIP						PTMOUSE_FLIP_OFF

#define PTMOUSE_DEFUALT_LEFT_POS					0
#define PTMOUSE_DEFUALT_RIGHT_POS					1
#define PTMOUSE_DEFUALT_UP_POS						2
#define PTMOUSE_DEFUALT_DOWN_POS					3
#define PTMOUSE_DEFUALT_BTN_POS						4

#define PTMOUSE_DEFAULT_RED_LEVEL					0xC0
#define PTMOUSE_DEFAULT_GREEN_LEVEL					0x3F
#define PTMOUSE_DEFAULT_BLUE_LEVEL					0xBC
#define PTMOUSE_DEFAULT_WHITE_LEVEL					0x00

#define PTMOUSE_MAX_DEVICES							4	// One each for R, G, B, and W.
#define PTMOUSE_RED_DEVICE_MINOR_NUMBER				0
#define PTMOUSE_GREEN_DEVICE_MINOR_NUMBER			1
#define PTMOUSE_BLUE_DEVICE_MINOR_NUMBER			2
#define PTMOUSE_WHITE_DEVICE_MINOR_NUMBER			3

#define PTMOUSE_MAX_OUTPUT_STRING_LENGTH			10
#define PTMOUSE_MAX_INPUT_STRING_LENGTH				6

static const char *ptMouse_device_names[PTMOUSE_MAX_DEVICES] = {
	"RED",
	"GREEN",
	"BLUE",
	"WHITE"
};

struct ptMouse_delayed_work;

struct ptMouse_data {
	struct delayed_work delayed_work;

	struct cdev cdev_list[PTMOUSE_MAX_DEVICES];

	struct workqueue_struct *workqueue_struct;
	dev_t dev_nr;
	int dev_major_number;
	struct class *my_class;

	uint8_t version_number;
	uint16_t chip_id;

	uint8_t work_rate_ms;
	uint8_t orientation;

	uint8_t swap;
	int8_t x_sign;
	int8_t y_sign;

	int16_t up_scale;
	int16_t down_scale;
	int16_t left_scale;
	int16_t right_scale;

	uint8_t lastButtonState;

	uint8_t red_level;
	uint8_t green_level;
	uint8_t blue_level;
	uint8_t white_level;

	unsigned short keycode[PTMOUSE_NUM_KEYCODES];
	struct i2c_client *i2c_client;
	struct input_dev *input_dev;
};


static const struct i2c_device_id ptMouse_i2c_device_id[] = {
	{ DEVICE_NAME, 0, },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ptMouse_i2c_device_id);

static const struct of_device_id ptMouse_of_device_id[] = {
	{ .compatible = COMPATIBLE_NAME, },
	{ }
};
MODULE_DEVICE_TABLE(of, ptMouse_of_device_id);


#endif
