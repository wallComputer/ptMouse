# ptMouse

ptMouse is a Mouse Device Driver for the [Pimoroni Trackball Breakbout](https://shop.pimoroni.com/products/trackball-breakout?variant=27672765038675) by [Pimoroni](https://shop.pimoroni.com). **The current version does not use the interrupt line but rather polls to devices to gather mouse data.**

The driver is named so, for  abbreviating the name of the original creators, [Pimoroni](https://shop.pimoroni.com) . The driver is mainly developed for [Raspberry Pi Foundation's](https://www.raspberrypi.org) [Raspberry Pi Zero 2 W](https://www.raspberrypi.com/products/raspberry-pi-zero-2-w/),[Raspberry Pi Zero W](https://www.raspberrypi.com/products/raspberry-pi-zero-w/), and [Raspberry Pi 3 Model B](https://www.raspberrypi.com/products/raspberry-pi-3-model-b/). 

## Features
The original [source code](https://github.com/pimoroni/trackball-python) by [Pimoroni](https://shop.pimoroni.com), is a good starting point from where this driver gathers information on the I2C registers and workings of the trackball. This driver offers the following features:

- Ability to use trackball as a mouse device.
- Ability to use trackball button as customizable input key.
- Ability to orient the trackball in 4 different orientations.
- Ability to control gain/scaling of each of the four directions, up, down, left, and right, independent of the orientation of the trackball.**
- Ability to change Button Keycode for the Button on the trackpad.


## Trackball Layout & Orientation

The trackball layout is shown below. This is the orientation Pimoroni uses to mention up, down, left, and right on the trackball. 


```
 *	  Pimoroni Trackball Layout
 *	+---------------------------+
 *	|       +-----------+       |
 *	|       |			|       |
 *	|       |  BTN_LEFT	|       |
 *	|       |			|       |
 *	|       +-----------+       |
 *	|                           |
 *	+------+             +------+
 *		   |             |
 *		   |3V3|D|C|I|GND|
 *		   |_____________|
```


## Pre-requisites, Download, and Installation.

### Pre-requisites

- Software Pre-requisites

  1. The Raspberry Pi must have kernel drivers installed on it.
     ```bash
     sudo apt install raspberrypi-kernel-headers
     ```
  2. The Pi must have its I2C Peripheral enabled. Run `sudo raspi-config`, then under `Interface Options`, enable `I2C`.

- Hardware Pre-rquisties: Trackball to Raspberry Pi Hardware connections. ***This Configuration does not use the Interrupt pin and it should be left untouched.*** 
  ```
  3.3V on Pi ----> 3.3V on Trackball Breakout.
  GND on Pi -----> GND on Trackball Breakout.
  SCL(Pi Pin 5/BCM GPIO 3) on Pi ----> SCL on Trackball Breakout.
  SDA(Pi Pin 3/BCM GPIO 2) on Pi ----> SDA on Trackball Breakout.
  ```

  3. Check pre-requisites with the command `i2cdetect -y 1`(one might need `sudo`). They trackball should be on the address `0x0A`.

### Download

Copy the Git repository from above and download it on your Pi, preferably on `/home/pi/` using the below command and change in to the folder.
```bash
git clone https://github.com/wallComputer/ptMouse.git
cd ptMouse/
```

### Installation
- To install the files, use the `installer.sh` file, followed by a reboot to get the driver working.
  ```bash
  source installer.sh
  ```
- To remove, use the `remover.sh` file. No reboot is necessay at this step.


After following the installation guide, the trackball should be working as a mouse. One can use any any screen to view the mouse at work, I personally used the [Pimoroni Display Hat Mini](https://shop.pimoroni.com/products/display-hat-mini?variant=39496084717651) on my Raspberry Pi Zero 2 W using [juj's](https://github.com/juj) `fbcp-ili9341` project. The driver also exposes the four writable LED registers for the RED, GREEN, BLUE, and WHITE LED as `/dev/ptMouse-RED`, `/dev/ptMouse-GREEN`, `/dev/ptMouse-BLUE`, `/dev/ptMouse-WHITE` character files. These can be written and read from. On read, these files provide the current intensity(brightness?) of the LED in a range from 0x00 to 0xFF. The registers can be written to as int values from 0 to 255, or in Hex Values from 0x00 to 0xFF.
```bash
$ echo 0x45 > /dev/ptMouse-BLUE
$ head -c5 /dev/ptMouse-RED
$ 0xC0
``` 

- (Optional) Pimoroni Display Hat Mini](https://shop.pimoroni.com/products/display-hat-mini?variant=39496084717651) has an ST7789V2 on it. Using it for the main console can be of great use. The below steps describe how to connect and control this LCD from the Pi. ***I WAS ABLE TO GET THIS WORKING ONLY AND ONLY WITH A RASPBERRY PI ZERO 2 W AND THAT TOO WITH 32 BIT BUSTER OS.***

  0. Hardware connections
	 Mate the display as a hat for the Pi.

	 Add the Trackball to the left hand side Breakout Extender. One can also use the Qwiic connector at the screen's bottom.

  1. Install git and cmake.
     ```bash
     sudo apt install git cmake
     ```
  2. Clone [Gadgetoid's](https://github.com/pimoroni/fbcp-ili9341.git) fork of [juj's](https://github.com/juj) project. For now, as the below changed by [Gadgetoid](https://github.com/Gadgetoid) are not merged with the main branch, you'll need to detac head and use the commit `6331bd41433d34919897dca483c0b5a060606598`. After that, create a directory `build` inside it, and change in to it. 
     ```bash
     git clone https://github.com/pimoroni/fbcp-ili9341.git
     cd fbcp-ili9341
	 git checkout 6331bd41433d34919897dca483c0b5a060606598
     mkdir build
     cd build/
     ```
  3. Inside the `build` directory, run the below `cmake` command.
     ```bash
	 cmake -DPIMORONI_DISPLAY_HAT_MINI=ON -DSPI_BUS_CLOCK_DIVISOR=40 -DDISPLAY_BREAK_ASPECT_RATIO_WHEN_SCALING=ON -DBACKLIGHT_CONTROL=ON -DSTATISTICS=0 ..
     ```
  4. Run a make command as
     ```bash
     make -j
     ```
  5. The result of the above file will be a `fbcp-ili9341` file. Place the absolute path of this file in the **last line but one** of `/etc/rc.local` as below **(assuming the absolute path is correct)**
     ```bash
     sudo /home/pi/fbcp-ili9341/build/fbcp-ili9341 &
     ```
  6. Place the below text in /boot/config.txt
     ```bash
     hdmi_group=2
     hdmi_mode=87
     hdmi_cvt=320 240 60 1 0 0 0
     hdmi_force_hotplug=1
     ```
  7. From `sudo raspi-config`, under `System Options`, set the `Boot / Auto Login` feature to `Desktop` or `Desktop Autologin`. 
  8. Reboot.
  9. On boot up, the LCD should start showing Desktop and if the trackball driver was installed, it should be able to control the cursor. 
## Customisation
	All the below customisations make use of module parameters, none of which can be edited without unloading and loading the driver back. This was a design choice, I do not like making the user go to root and change anything on a computer. Hence to make these changes, one can edit the `installer.sh` and change the module parameters.
 - For changing the Orientation, one can use the `orientation` module parameter. The range of values this parameter takes is 0,1,2, and 3. 0 signifies the orientation as shown above, while each next number rotates the trackball 90 degrees clockwise. To be used with Display Hat mini, one can see it requires orientation of 270 degrees, and hence the `installer.sh` sets the orientation to `3`.
 - For changing the gain/scales of the cursor motion, one can use the `up_scale`, `down_scale`, `left_scale`, `right_scale` module parameters. These parameters are aligned to the axis of a display, that is, top left corner is the origin to the screen, left to right is increasing X axis, and top to bottom is increasing Y axis. So increasing `up_scale`, will make rolling the trackball (in correct orientation) in the upward direction quicker. The scales are themselves stretched from `100`. This way, to decrease the quickness of cursor motion in one direction, you can reduce the `_scale` of that direction below `100`. For the Display Hat Mini, from personal use it was found a `500` scale for up and down directions and a `1000` scale for left and right directions was useful.
 - For changing the default button key on the trackball, use `button_keycode` module parameter. One will need to provide the appropriate scancode from [input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h) in decimal format. For example, for `BTN_LEFT` which has hexadecimal value `0x110` or decimal value `272`, use `button_keycode=272`, similarly for `BTN_RIGHT` which has hecadecimal value `0x111` or decimal value `273`, use `button_keycode=273`.
## Known Issues

  - The BIGGEST FLAW at the moment is that the driver does not use the interrupt line of the trackball. Unfortunately, it seems to be an open and unaddressed issue. From analysis, it was found that the interrupt line toggles every time the device is being read from, whether the trackball is touched or not, even when the device was reset and interrupt register was cleared. If this issue is solved, the driver will be moved to a IRQ based system. The current one works by polling the trackball every 20ms or so. There is the `work_rate_ms` module parameter which is used to set this, but it's not the most straight forward way to use. If you know the cause for this issue, please reach out.
  - The trackball only presents RGBW interface. More sophesticated trackballs with LEDs have HSB interface too. This will be worked on in future iterations.
  - The trackball only works in upward facing orientation. To set it in more orientations, specially those around curved edges and at non right angled positions, one might need to use a bit of Mathematics to find the correct X and Y axis relative motions for each reading. 
  - I've never encountered as many kernel panics in my entire work than I did while making this driver. This is not to say the driver is faulty. I'm releasing it only after testing it for 100s of times. Just that I faced ~50 kernel panics while making this driver. If you face an issue of that sort, please report and I'd like to help.

## Future Work
 - Solve the BIGGEST FLAW, Use interrupt line and not poll the device.
 - Provide HSB interface for the LEDs.
 - Support more orientations for the trackball.
 - Provide support for multiple trackballs in one driver to make a small two handed (finger?) joystick like experience.
 - Keep removing any and all cases of kernel panic.

## Contributing
Pull requests are welcome. 

## Special thanks
 - [Pimoroni](https://shop.pimoroni.com) & [Gadgetoid](https://github.com/Gadgetoid) for the amazing product and python libraries. Please buy more of their new awesome stuff and join their discord for amazing upcoming stuff.
 - [juj's](https://github.com/juj) `fbcp-ili9341` project for making small size screens so easy to use! 


## License
The code itself is LGPL-2.0 though I'm not going to stick it to you, I just chose what I saw, if you want to do awesome stuff useful to others with it without harming anyone, go for it and have fun just as I did.