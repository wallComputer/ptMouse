// SPDX-License-Identifier: GPL-2.0-only
/*
 * Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.
 * ptMouse_i2cHelper.h: I2C Functions C file.
 */

#include "ptMouse_i2cHelper.h"

extern int ptMouse_write(struct i2c_client *i2c_client, uint8_t deviceAddress, uint8_t registerAddress, const uint8_t *buffer, uint8_t bufferSize)
{
	int returnValue;

	switch (deviceAddress) {
	case PTMOUSE_I2C_ADDRESS:
		if (bufferSize == sizeof(uint8_t)) {
			returnValue = i2c_smbus_write_byte_data(i2c_client, registerAddress, *buffer);
			if (returnValue != 0) {
				dev_err(&i2c_client->dev, "%s Could not write byte to register 0x%02X, Error: %d\n", __func__, registerAddress, returnValue);
				return returnValue;
			}
#if (DEBUG_LEVEL & DEBUG_LEVEL_RW)
			dev_info(&i2c_client->dev, "%s Wrote data: 0x%02X to register 0x%02X\n", __func__, *buffer, registerAddress);
#endif
		} else if (bufferSize == sizeof(uint8_t)*2) {
			returnValue = i2c_smbus_write_word_data(i2c_client, registerAddress, *buffer);
			if (returnValue != 0) {
				dev_err(&i2c_client->dev, "%s Could not write word to register 0x%02X, Error: %d\n", __func__, registerAddress, returnValue);
				return returnValue;
			}
#if (DEBUG_LEVEL & DEBUG_LEVEL_RW)
			dev_info(&i2c_client->dev, "%s Wrote data: 0x%02X 0x%02X  to register 0x%02X\n", __func__, *buffer, *(buffer+1), registerAddress);
#endif
		} else if (bufferSize == sizeof(uint8_t)*4) {
			returnValue = i2c_smbus_write_block_data(i2c_client, registerAddress, 4, buffer);
			if (returnValue != 0) {
				dev_err(&i2c_client->dev, "%s Could not write block to register 0x%02X, Error: %d\n", __func__, registerAddress, returnValue);
				return returnValue;
			}
#if (DEBUG_LEVEL & DEBUG_LEVEL_RW)
			dev_info(&i2c_client->dev, "%s Wrote data: 0x%02X 0x%02X 0x%02X 0x%02X to register 0x%02X\n", __func__, *buffer, *(buffer+1), *(buffer+2), *(buffer+3), registerAddress);
#endif
		}
		break;
	default:
		return 0;
	}
	return 0;
}
extern int ptMouse_read(struct i2c_client *i2c_client, uint8_t deviceAddress, uint8_t registerAddress, uint8_t *buffer, uint8_t bufferSize)
{
	int returnValue;

	switch (deviceAddress) {
	case PTMOUSE_I2C_ADDRESS:
	{
		if (bufferSize == sizeof(uint8_t)) {
			returnValue = i2c_smbus_read_byte_data(i2c_client, registerAddress);
			if (returnValue < 0) {
				dev_err(&i2c_client->dev, "%s Could not read byte from register 0x%02X, error: %d\n", __func__, registerAddress, returnValue);
				return returnValue;
			}
			*buffer = (uint8_t)(returnValue & 0xFF);
#if (DEBUG_LEVEL & DEBUG_LEVEL_RW)
			dev_info(&i2c_client->dev, "%s Read data: 0x%02X from register 0x%02X\n", __func__, *buffer, registerAddress);
#endif
		} else if (bufferSize == sizeof(uint8_t)*2) {
			returnValue = i2c_smbus_read_word_data(i2c_client, registerAddress);
			if (returnValue < 0) {
				dev_err(&i2c_client->dev, "%s Could not read word from register 0x%02X, error: %d\n", __func__, registerAddress, returnValue);
				return returnValue;
			}
			*buffer = (uint8_t)((returnValue & 0xFF00) >> 8);
			*(buffer+1) = (uint8_t)(returnValue & 0xFF);
#if (DEBUG_LEVEL & DEBUG_LEVEL_RW)
			dev_info(&i2c_client->dev, "%s Read data: 0x%02X and 0x%02X from register 0x%02X\n", __func__, *buffer, *(buffer+1), registerAddress);
#endif
		} else {
			returnValue = i2c_smbus_read_i2c_block_data(i2c_client, registerAddress, bufferSize/sizeof(uint8_t), buffer);
			if (returnValue < 0) {
				dev_err(&i2c_client->dev, "%s Could not read block from register 0x%02X, error: %d\n", __func__, registerAddress, returnValue);
				return returnValue;
			}
		}
		break;
	}
	default:
		return 0;
	}
	return 0;
}
