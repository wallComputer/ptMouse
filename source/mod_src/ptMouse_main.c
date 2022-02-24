// SPDX-License-Identifier: GPL-2.0-only
/*
 * Convert Pimoroni Trackball in to a mouse. Software written by wallComputer.
 * ptMouse.c: Main C File.
 */

#include "ptMouse_main.h"


static struct i2c_client *fileIO_i2c_client; /* This is needed for getting access to other structs in ptMouse_exit function. */
											/* If there is a better way to access them, do let me know.*/

static int button_keycode = BTN_LEFT;
module_param(button_keycode, int, 0);

static int work_rate_ms = PTMOUSE_DEFAULT_WORK_RATE;
module_param(work_rate_ms, int, 0);

static int orientation = PTMOUSE_DEFAULT_ORIENTATION;
module_param(orientation, int, 0);

static int up_scale = PTMOUSE_DEFAULE_SCALE_FACTOR;
module_param(up_scale, int, 0);

static int down_scale = PTMOUSE_DEFAULE_SCALE_FACTOR;
module_param(down_scale, int, 0);

static int left_scale = PTMOUSE_DEFAULE_SCALE_FACTOR;
module_param(left_scale, int, 0);

static int right_scale = PTMOUSE_DEFAULE_SCALE_FACTOR;
module_param(right_scale, int, 0);

static atomic_t keepWorking = ATOMIC_INIT(1);

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
static ktime_t startTime, stopTime, lastStartTime, lastStopTime;
#endif

static void ptMouse_work_handler(struct work_struct *work_struct)
{
	struct ptMouse_data *ptMouse_data;
	uint8_t registerU8Value;
	uint8_t u8X5Values[5];
	uint8_t currentButtonState;
	int returnValue;
	int rel_X, rel_Y;

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	lastStartTime = startTime;
	startTime = ktime_get_ns();
#endif
	if (atomic_read(&keepWorking) == 1) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
		pr_info("%s Done with QUEUE.\n", __func__);
#endif
		return;
	}
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Doing Queue now.\n", __func__);
#endif
	ptMouse_data = container_of(work_struct, struct ptMouse_data, delayed_work.work);
	returnValue = ptMouse_read(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_INTERRUPT_REG, &registerU8Value, sizeof(uint8_t));
	if (returnValue < 0) {
		dev_err(&ptMouse_data->i2c_client->dev, "%s Could not read REG_INT. Error: %d\n", __func__, returnValue);
		return;
	}
	if (registerU8Value & PTMOUSE_MASK_INTERRUPT_TRIGGER) {
		returnValue = ptMouse_read(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_LEFT_MOTION_REG, u8X5Values, sizeof(uint8_t)*5);
		if (returnValue < 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not read PTMOUSE_LEFT_MOTION_REG. Error: %d\n", __func__, returnValue);
			return;
		}
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
		dev_info(&ptMouse_data->i2c_client->dev, "%s DEFAULT:: LEFT:%d RIGHT:%d UP:%d DOWN:%d BUTTON:%d\n", __func__,
													u8X5Values[PTMOUSE_DEFUALT_LEFT_POS], u8X5Values[PTMOUSE_DEFUALT_RIGHT_POS],
													u8X5Values[PTMOUSE_DEFUALT_UP_POS], u8X5Values[PTMOUSE_DEFUALT_DOWN_POS],
													u8X5Values[PTMOUSE_DEFUALT_BTN_POS]);
#endif
		if (ptMouse_data->swap) {
			rel_X = ptMouse_data->x_sign*(u8X5Values[PTMOUSE_DEFUALT_UP_POS]*ptMouse_data->left_scale - u8X5Values[PTMOUSE_DEFUALT_DOWN_POS]*ptMouse_data->right_scale)/PTMOUSE_DEFAULE_SCALE_FACTOR;
			rel_Y = ptMouse_data->y_sign*(u8X5Values[PTMOUSE_DEFUALT_RIGHT_POS]*ptMouse_data->down_scale - u8X5Values[PTMOUSE_DEFUALT_LEFT_POS]*ptMouse_data->up_scale)/PTMOUSE_DEFAULE_SCALE_FACTOR;
		} else {
			rel_X = ptMouse_data->x_sign*(u8X5Values[PTMOUSE_DEFUALT_RIGHT_POS]*ptMouse_data->right_scale - u8X5Values[PTMOUSE_DEFUALT_LEFT_POS]*ptMouse_data->left_scale)/PTMOUSE_DEFAULE_SCALE_FACTOR;
			rel_Y = ptMouse_data->y_sign*(u8X5Values[PTMOUSE_DEFUALT_UP_POS]*ptMouse_data->down_scale - u8X5Values[PTMOUSE_DEFUALT_DOWN_POS]*ptMouse_data->up_scale)/PTMOUSE_DEFAULE_SCALE_FACTOR;
		}
		currentButtonState = u8X5Values[PTMOUSE_DEFUALT_BTN_POS]>>7; // Use only the Button state of pressed or released.
		if (currentButtonState ^ ptMouse_data->lastButtonState)
			input_report_key(ptMouse_data->input_dev, ptMouse_data->keycode[0], currentButtonState == PTMOUSE_SWITCH_PRESSED_STATE);
		ptMouse_data->lastButtonState = currentButtonState;
		input_report_rel(ptMouse_data->input_dev, REL_X, rel_X);
		input_report_rel(ptMouse_data->input_dev, REL_Y, rel_Y);
		input_sync(ptMouse_data->input_dev);
	}

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	lastStopTime = stopTime;
	stopTime = ktime_get_ns();
#endif
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	dev_info(&ptMouse_data->i2c_client->dev, "%s startTime: %lld stopTime: %lld startDiff: %lld, stopDiff: %lld taskTime: %lld\n", __func__,
															startTime, stopTime, startTime-lastStartTime, stopTime-lastStopTime, stopTime-startTime);
#endif

	queue_delayed_work(ptMouse_data->workqueue_struct, &ptMouse_data->delayed_work, msecs_to_jiffies(ptMouse_data->work_rate_ms));
}

static int ptMouse_queue_work(struct ptMouse_data *ptMouse_data)
{
	INIT_DELAYED_WORK(&ptMouse_data->delayed_work, ptMouse_work_handler);
	atomic_set(&keepWorking, 0);
	return queue_delayed_work(ptMouse_data->workqueue_struct, &ptMouse_data->delayed_work, msecs_to_jiffies(ptMouse_data->work_rate_ms));
}

static void ptMouse_set_init_conditons(struct ptMouse_data *ptMouse_data)
{
	int returnValue;

	if (work_rate_ms <= PTMOUSE_MINIMUM_WORK_RATE || work_rate_ms >= PTMOUSE_MAXIMUM_WORK_RATE)
		ptMouse_data->work_rate_ms = PTMOUSE_DEFAULT_WORK_RATE;
	else
		ptMouse_data->work_rate_ms = work_rate_ms;

	if ((orientation != PTMOUSE_ORIENTATION_0) && (orientation != PTMOUSE_ORIENTATION_90) && (orientation != PTMOUSE_ORIENTATION_180) && (orientation != PTMOUSE_ORIENTATION_270))
		ptMouse_data->orientation = PTMOUSE_DEFAULT_ORIENTATION;
	else
		ptMouse_data->orientation = orientation;

	//THERE MUST BE AN ORDER TO HOW THIS IS DONE. HERE IT'S DONE EMPIRICALLY. If you are reading this and know the math behind it, make a pull request and let me know.
	//Set values for 270 degree rotation (as used in base case of Omnigrammer.)
	ptMouse_data->swap = 1;
	ptMouse_data->x_sign = -1;
	ptMouse_data->y_sign = -1;
	ptMouse_data->up_scale = down_scale;
	ptMouse_data->down_scale = up_scale;
	ptMouse_data->left_scale = left_scale;
	ptMouse_data->right_scale = right_scale;
	ptMouse_data->lastButtonState = 0x00;

	if (ptMouse_data->orientation == PTMOUSE_ORIENTATION_0) { // Same orientation as default Trackball orientation.
		ptMouse_data->swap = 0;
		ptMouse_data->x_sign = 1;
		ptMouse_data->y_sign = -1;
	} else if (ptMouse_data->orientation == PTMOUSE_ORIENTATION_90) { // 90 degrees rotated from default Trackball orientation.
		ptMouse_data->swap = 1;
		ptMouse_data->x_sign = 1;
		ptMouse_data->y_sign = 1;
		ptMouse_data->up_scale = up_scale;
		ptMouse_data->down_scale = down_scale;
		ptMouse_data->left_scale = right_scale;
		ptMouse_data->right_scale = left_scale;
	}  else if (ptMouse_data->orientation == PTMOUSE_ORIENTATION_180) { // 180 degrees rotated from default Trackball orientation.
		ptMouse_data->swap = 0;
		ptMouse_data->x_sign = -1;
		ptMouse_data->y_sign = 1;
		ptMouse_data->up_scale = up_scale;
		ptMouse_data->down_scale = down_scale;
		ptMouse_data->left_scale = right_scale;
		ptMouse_data->right_scale = left_scale;
	}
	ptMouse_data->red_level = PTMOUSE_DEFAULT_RED_LEVEL;
	ptMouse_data->green_level = PTMOUSE_DEFAULT_GREEN_LEVEL;
	ptMouse_data->blue_level = PTMOUSE_DEFAULT_BLUE_LEVEL;
	ptMouse_data->white_level = PTMOUSE_DEFAULT_WHITE_LEVEL;

	returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_RED_LED_REGISTER, &ptMouse_data->red_level, sizeof(uint8_t));
	if (returnValue != 0) {
		dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write R registers from PTMOUSE_RED_LED_REGISTER. Error: %d\n", __func__, returnValue);
		return;
	}
	returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_GREEN_LED_REGISTER, &ptMouse_data->green_level, sizeof(uint8_t));
	if (returnValue != 0) {
		dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write G registers from PTMOUSE_GREEN_LED_REGISTER. Error: %d\n", __func__, returnValue);
		return;
	}
	returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_BLUE_LED_REGISTER, &ptMouse_data->blue_level, sizeof(uint8_t));
	if (returnValue != 0) {
		dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write B registers from PTMOUSE_BLUE_LED_REGISTER. Error: %d\n", __func__, returnValue);
		return;
	}
	returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_WHITE_LED_REGISTER, &ptMouse_data->white_level, sizeof(uint8_t));
	if (returnValue != 0) {
		dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write W registers from PTMOUSE_WHITE_LED_REGISTER. Error: %d\n", __func__, returnValue);
		return;
	}
}

static int ptMouse_fops_open(struct inode *inode, struct file *file)
{
	uint8_t minor_number;
	struct ptMouse_data *ptMouse_data;

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Inside fops open.\n", __func__);
#endif

	minor_number = inode->i_rdev;
	ptMouse_data = container_of(inode->i_cdev, struct ptMouse_data, cdev_list[minor_number]);
	file->private_data = ptMouse_data;

	return 0;
}

static int ptMouse_fops_release(struct inode *inode, struct file *file)
{

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Inside fops release.\n", __func__);
#endif

	return 0;
}

static ssize_t ptMouse_fops_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
	uint8_t minor_number;
	struct ptMouse_data *ptMouse_data;
	int to_copy;
	size_t copy_size;
	int not_copied;
	int delta;
	int returnValue;
	char out_string[PTMOUSE_MAX_OUTPUT_STRING_LENGTH];
	uint8_t rgbw_value;


	ptMouse_data = file->private_data;
	minor_number = MINOR(file->f_path.dentry->d_inode->i_rdev);
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	dev_info(&ptMouse_data->i2c_client->dev, "%s Inside fops read of node %s\n", __func__, ptMouse_device_names[minor_number]);
#endif


	switch (minor_number) {
	case PTMOUSE_RED_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_read(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_RED_LED_REGISTER, &rgbw_value, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not read PTMOUSE_RED_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		copy_size = 5;
		break;
	case PTMOUSE_GREEN_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_read(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_GREEN_LED_REGISTER, &rgbw_value, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not read PTMOUSE_GREEN_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		copy_size = 5;
		break;
	case PTMOUSE_BLUE_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_read(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_BLUE_LED_REGISTER, &rgbw_value, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not read PTMOUSE_BLUE_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		copy_size = 5;
		break;
	case PTMOUSE_WHITE_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_read(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_WHITE_LED_REGISTER, &rgbw_value, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not read PTMOUSE_WHITE_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		copy_size = 5;
		break;
	default:
		break;
	}


	to_copy = min(copy_size, count);
	snprintf(out_string, sizeof(out_string), "0x%02X\n", rgbw_value);


	not_copied = copy_to_user(user_buffer, out_string, to_copy);

	delta = to_copy - not_copied;

	return delta;
}

static int input_string_to_registerValue(uint8_t *data_buffer, int buffer_size, uint8_t *registerValue)
{
	uint8_t is_hex = 0;
	uint8_t looper = 0;
	uint16_t value = 0;
	uint8_t highPos = 0;
	uint8_t lowPos = 0;

	if ((data_buffer[0] < '0' || data_buffer[0] > '9') || buffer_size == 0) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
		pr_info("%s FALSE 1.\n", __func__);
#endif
		return -1;
	}
	if (data_buffer[1] == 'X' || data_buffer[1] == 'x') {
		if (buffer_size != 5) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
			pr_info("%s FALSE 2.\n", __func__);
#endif
			return -1;
		}
		if (data_buffer[0] != '0') {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
			pr_info("%s FALSE 3.\n", __func__);
#endif
			return -1;
		}
		if (!(data_buffer[2] >= '0' && data_buffer[2] <= '9') && !(data_buffer[2] >= 'a' && data_buffer[2] <= 'f') && !(data_buffer[2] >= 'A' && data_buffer[2] <= 'F')) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
			pr_info("%s FALSE 4. %d %d %d\n", __func__,
			!(data_buffer[2] >= '0' && data_buffer[2] <= '9'),
			!(data_buffer[2] >= 'a' && data_buffer[2] <= 'f'),
			!(data_buffer[2] >= 'A' && data_buffer[2] <= 'F'));
#endif
			return -1;
		}
		if (!(data_buffer[3] >= '0' && data_buffer[3] <= '9') && !(data_buffer[3] >= 'a' && data_buffer[3] <= 'f') && !(data_buffer[3] >= 'A' && data_buffer[3] <= 'F')) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
			pr_info("%s FALSE 5. %d %d %d\n", __func__,
			!(data_buffer[3] >= '0' && data_buffer[3] <= '9'),
			!(data_buffer[3] >= 'a' && data_buffer[3] <= 'f'),
			!(data_buffer[3] >= 'A' && data_buffer[3] <= 'F'));
#endif
			return -1;
		}
		is_hex = 1;
	}

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s is_hex: %d\n", __func__, is_hex);
#endif

	if (is_hex) {
		highPos = data_buffer[2];
		lowPos = data_buffer[3];
		if (highPos >= 'a' && highPos <= 'f')
			highPos = highPos - 'a' + 10;
		else if (highPos >= 'A' && highPos <= 'F')
			highPos = highPos - 'A' + 10;
		else
			highPos = highPos - '0';
		if (lowPos >= 'a' && lowPos <= 'f')
			lowPos = lowPos - 'a' + 10;
		else if (lowPos >= 'A' && lowPos <= 'F')
			lowPos = lowPos - 'A' + 10;
		else
			lowPos = lowPos - '0';

		*registerValue = highPos*16 + lowPos;
		return 0;
	}

	for (looper = 0; looper < buffer_size-1; ++looper) {
		if (data_buffer[looper] < '0' || data_buffer[looper] > '9')
			return -1;
		value = value * 10 + (data_buffer[looper] - '0');
	}
	if (value > (0xFF)) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
		pr_info("%s FALSE 6.\n", __func__);
#endif
		return -1;
	}
	*registerValue = (uint8_t)value;
	return 0;

}

static ssize_t ptMouse_fops_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offset)
{
	uint8_t data_buffer[PTMOUSE_MAX_INPUT_STRING_LENGTH];
	uint8_t minor_number;
	uint8_t registerValue;
	int returnValue;
	int to_copy;
	int not_copied;
	int delta;
	struct ptMouse_data *ptMouse_data;


	ptMouse_data = file->private_data;
	minor_number = MINOR(file->f_path.dentry->d_inode->i_rdev);
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	dev_info(&ptMouse_data->i2c_client->dev, "%s Inside fops write of node %s\n", __func__, ptMouse_device_names[minor_number]);
#endif

	if (count >= PTMOUSE_MAX_INPUT_STRING_LENGTH) {
		dev_warn(&ptMouse_data->i2c_client->dev, "%s Input Size of %d is too long! Maximum Allowed: %d\n", __func__, count, PTMOUSE_MAX_INPUT_STRING_LENGTH);
		return -1;
	}

	to_copy = min(count, sizeof(data_buffer));

	not_copied = copy_from_user(data_buffer, user_buffer, to_copy);

	data_buffer[to_copy] = '\0';

	delta = to_copy - not_copied;
	returnValue = input_string_to_registerValue(data_buffer, to_copy, &registerValue);
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	dev_info(&ptMouse_data->i2c_client->dev, "%s Inside fops write of node %s To Copy: %d Data: %s returnValue: %d registerValue: %d\n", __func__, ptMouse_device_names[minor_number], to_copy, data_buffer, returnValue, registerValue);
#endif
	if (returnValue != 0) {
		dev_warn(&ptMouse_data->i2c_client->dev, "%s Input is not a valid Hexadecimal or decimal.!\n", __func__);
		return -1;
	}

	switch (minor_number) {
	case PTMOUSE_RED_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_RED_LED_REGISTER, &registerValue, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write to PTMOUSE_RED_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		break;
	case PTMOUSE_GREEN_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_GREEN_LED_REGISTER, &registerValue, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write to PTMOUSE_GREEN_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		break;
	case PTMOUSE_BLUE_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_BLUE_LED_REGISTER, &registerValue, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write to PTMOUSE_BLUE_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		break;
	case PTMOUSE_WHITE_DEVICE_MINOR_NUMBER:
		returnValue = ptMouse_write(ptMouse_data->i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_WHITE_LED_REGISTER, &registerValue, sizeof(uint8_t));
		if (returnValue != 0) {
			dev_err(&ptMouse_data->i2c_client->dev, "%s Could not write to PTMOUSE_WHITE_LED_REGISTER. Error: %d\n", __func__, returnValue);
			return -1;
		}
		break;
	}

	return delta;
}

static const struct file_operations ptMouse_fops = {
	.owner = THIS_MODULE,
	.open = ptMouse_fops_open,
	.release = ptMouse_fops_release,
	.read = ptMouse_fops_read,
	.write = ptMouse_fops_write
};

static int ptMouse_data_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0666);
	return 0;
}

static void ptMouse_create_char_devices(struct ptMouse_data *ptMouse_data)
{
	int returnValue;
	uint8_t looper;

	returnValue = alloc_chrdev_region(&ptMouse_data->dev_nr, 0, PTMOUSE_MAX_DEVICES, DEVICE_NAME);

	ptMouse_data->dev_major_number = MAJOR(ptMouse_data->dev_nr);

	ptMouse_data->my_class = class_create(THIS_MODULE, DEVICE_NAME);
	ptMouse_data->my_class->dev_uevent = ptMouse_data_uevent;

	for (looper = 0; looper < PTMOUSE_MAX_DEVICES; ++looper) {
		cdev_init(&ptMouse_data->cdev_list[looper], &ptMouse_fops);
		ptMouse_data->cdev_list[looper].owner = THIS_MODULE;

		cdev_add(&ptMouse_data->cdev_list[looper], MKDEV(ptMouse_data->dev_major_number, looper), 1);

		device_create(ptMouse_data->my_class, NULL, MKDEV(ptMouse_data->dev_major_number, looper), NULL, "%s-%s", DEVICE_NAME, ptMouse_device_names[looper]);
	}


}

static int ptMouse_probe(struct i2c_client *i2c_client, const struct i2c_device_id *id)
{
	struct device *dev = &i2c_client->dev;
	struct ptMouse_data *ptMouse_data;
	struct input_dev *input;
	int returnValue = 0;
	uint8_t registerU8Value = 0x00;
	uint8_t registerU16Value[2];

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	dev_info(dev, "%s Probing ptMouse.\n", __func__);
#endif
	ptMouse_data = devm_kzalloc(dev, sizeof(struct ptMouse_data), GFP_KERNEL);
	if (!ptMouse_data)
		return -ENOMEM;
	ptMouse_data->i2c_client = i2c_client;
	i2c_set_clientdata(i2c_client, ptMouse_data);
	fileIO_i2c_client = i2c_client;
	memcpy(ptMouse_data->keycode, ptMouse_keycodes, sizeof(ptMouse_data->keycode));

	ptMouse_data->keycode[0] = button_keycode;

	registerU8Value = registerU8Value | PTMOUSE_CONTROL_RESET_MASK;
	returnValue = ptMouse_write(i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_CONTROL_REG, &registerU8Value, sizeof(uint8_t));
	if (returnValue != 0) {
		dev_err(dev, "%s Could not reset PTMOUSE_CONTROL_REG. Error: %d\n", __func__, returnValue);
		return returnValue;
	}
	msleep(50);

	returnValue = ptMouse_read(i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_VERSION_REG, &registerU8Value, sizeof(uint8_t));
	if (returnValue != 0) {
		dev_err(dev, "%s Could not read PTMOUSE_VERSION_REG. Error: %d\n", __func__, returnValue);
		return returnValue;
	}
	ptMouse_data->version_number = registerU8Value;
	returnValue = ptMouse_read(i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_CHIP_ID_LOW_REG, registerU16Value, sizeof(uint8_t)*2);
	if (returnValue != 0) {
		dev_err(dev, "%s Could not read PTMOUSE_CHIP_ID_LOW_REG. Error: %d\n", __func__, returnValue);
		return returnValue;
	}
	ptMouse_data->chip_id = (registerU16Value[0]<<8 | registerU16Value[1]);
	if (ptMouse_data->chip_id != PTMOUSE_CHIP_ID_VALUE || ptMouse_data->version_number != PTMOUSE_VERSION_VALUE) {
		dev_warn(dev, "%s Pimoroni Version Number or Chip ID do not match! Expected: (0x%04X 0x%02X) Received: (0x%04X 0x%02X). Exiting!\n", __func__,
			PTMOUSE_CHIP_ID_VALUE, PTMOUSE_VERSION_VALUE,
			ptMouse_data->chip_id, ptMouse_data->version_number);
			return -1;
	}
	dev_info(dev, "%s PTMOUSE CHIP ID: 0x%04X PTMOUSE VERSION: 0x%02X\n", __func__, ptMouse_data->chip_id, ptMouse_data->version_number);

	ptMouse_set_init_conditons(ptMouse_data);

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	dev_info(dev, "%s work_rate: %d mtoF: %lu Orientation: %d Swap: %d x_sign: %d y_sign: %d\n", __func__,
											ptMouse_data->work_rate_ms,
											msecs_to_jiffies(ptMouse_data->work_rate_ms),
											ptMouse_data->orientation,
											ptMouse_data->swap,
											ptMouse_data->x_sign,
											ptMouse_data->y_sign);
#endif

	ptMouse_create_char_devices(ptMouse_data);

	input = devm_input_allocate_device(dev);
	if (!input) {
		dev_err(dev, "%s Could not devm_input_allocate_device ptMouse. Error: %d\n", __func__, returnValue);
		return -ENOMEM;
	}
	ptMouse_data->input_dev = input;

	input->name = i2c_client->name;
	input->id.bustype	= PTMOUSE_BUS_TYPE;
	input->id.vendor	= PTMOUSE_VENDOR_ID;
	input->id.product	= PTMOUSE_PRODUCT_ID;
	input->id.version	= PTMOUSE_VERSION_ID;
	input->keycode		= ptMouse_data->keycode;
	input->keycodesize	= sizeof(ptMouse_data->keycode[0]);
	input->keycodemax	= ARRAY_SIZE(ptMouse_data->keycode);

	__set_bit(ptMouse_data->keycode[0], input->keybit);
	__clear_bit(KEY_RESERVED, input->keybit);
	__set_bit(EV_REP, input->evbit);
	__set_bit(EV_KEY, input->evbit);
	input_set_capability(input, EV_MSC, MSC_SCAN);

	input_set_capability(input, EV_REL, REL_X);
	input_set_capability(input, EV_REL, REL_Y);

	ptMouse_data->workqueue_struct = create_singlethread_workqueue("ptMouse_workqueue");
	if (ptMouse_data->workqueue_struct == NULL) {
		dev_err(dev, "%s Could not create_singlethread_workqueue.", __func__);
		return -ENOMEM;
	}
	ptMouse_queue_work(ptMouse_data);


	returnValue = input_register_device(input);
	if (returnValue != 0) {
		dev_err(dev, "%s Could not register input device. Error: %d\n", __func__, returnValue);
		return returnValue;
	}

	return returnValue;
}

static void ptMouse_shutdown(struct i2c_client *i2c_client)
{
	int returnValue;
	uint8_t registerValue = 0x00;

	returnValue = ptMouse_write(i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_RED_LED_REGISTER, &registerValue, sizeof(uint8_t));
	if (returnValue != 0)
		dev_err(&i2c_client->dev, "%s Could not write to PTMOUSE_RED_LED_REGISTER. Error: %d\n", __func__, returnValue);
	returnValue = ptMouse_write(i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_GREEN_LED_REGISTER, &registerValue, sizeof(uint8_t));
	if (returnValue != 0)
		dev_err(&i2c_client->dev, "%s Could not write to PTMOUSE_GREEN_LED_REGISTER. Error: %d\n", __func__, returnValue);
	returnValue = ptMouse_write(i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_BLUE_LED_REGISTER, &registerValue, sizeof(uint8_t));
	if (returnValue != 0)
		dev_err(&i2c_client->dev, "%s Could not write to PTMOUSE_BLUE_LED_REGISTER. Error: %d\n", __func__, returnValue);
	returnValue = ptMouse_write(i2c_client, PTMOUSE_I2C_ADDRESS, PTMOUSE_WHITE_LED_REGISTER, &registerValue, sizeof(uint8_t));
	if (returnValue != 0)
		dev_err(&i2c_client->dev, "%s Could not write to PTMOUSE_WHITE_LED_REGISTER. Error: %d\n", __func__, returnValue);

}

static struct i2c_driver ptMouse_driver = {
	.driver = {
		.name = DEVICE_NAME,
		.of_match_table = ptMouse_of_device_id,
	},
	.probe			= ptMouse_probe,
	.shutdown		= ptMouse_shutdown,
	.id_table		= ptMouse_i2c_device_id,
};

static int __init ptMouse_init(void)
{
	int returnValue = 0;

	returnValue = i2c_add_driver(&ptMouse_driver);
	if (returnValue != 0) {
		pr_err("%s Could not initialise ptMouse driver! Error: %d\n", __func__, returnValue);
		return returnValue;
	}
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Initalised ptMouse.\n", __func__);
#endif
	return returnValue;
}
module_init(ptMouse_init);

static void __exit ptMouse_exit(void)
{
	uint8_t looper;
	struct ptMouse_data *ptMouse_data;

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Exiting ptMouse.\n", __func__);
#endif
	ptMouse_data = i2c_get_clientdata(fileIO_i2c_client);
	atomic_set(&keepWorking, 1);
	flush_workqueue(ptMouse_data->workqueue_struct);
	msleep(500);
	destroy_workqueue(ptMouse_data->workqueue_struct);
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Shutting Down LEDs.\n", __func__);
#endif
	ptMouse_shutdown(ptMouse_data->i2c_client);

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Destroying Devices.\n", __func__);
#endif
	for (looper = 0; looper < PTMOUSE_MAX_DEVICES; ++looper) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
		pr_info("%s Destroying Device %d\n", __func__, looper);
#endif
		device_destroy(ptMouse_data->my_class, MKDEV(ptMouse_data->dev_major_number, looper));
	}

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Destroying class.\n", __func__);
#endif
	class_destroy(ptMouse_data->my_class);

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Deleting cdev_list.\n", __func__);
#endif
	for (looper = 0; looper < PTMOUSE_MAX_DEVICES; ++looper) {
#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
		pr_info("%s Deleting cdev %d\n", __func__, looper);
#endif
		cdev_del(&ptMouse_data->cdev_list[looper]);
	}

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Unregistering chrdev region.\n", __func__);
#endif
	unregister_chrdev_region(MKDEV(ptMouse_data->dev_major_number, 0), MINORMASK);

#if (DEBUG_LEVEL & DEBUG_LEVEL_FE)
	pr_info("%s Deleting i2c driver.\n", __func__);
#endif
	i2c_del_driver(&ptMouse_driver);
	pr_info("%s Deleted i2c_driver and exiting.\n", __func__);
}
module_exit(ptMouse_exit);
