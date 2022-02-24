/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.
 * ptMouse_i2cHelper.h: I2C Functions H file.
 */
#ifndef PTMOUSE_I2CHELPER_H_
#define PTMOUSE_I2CHELPER_H_

#include <linux/i2c.h>
#include "ptMouse_registers.h"
#include "debug_levels.h"

extern int ptMouse_write(struct i2c_client *i2c_client, uint8_t deviceAddress, uint8_t registerAddress, const uint8_t *buffer, uint8_t bufferSize);
extern int ptMouse_read(struct i2c_client *i2c_client, uint8_t deviceAddress, uint8_t registerAddress, uint8_t *buffer, uint8_t bufferSize);
#endif
