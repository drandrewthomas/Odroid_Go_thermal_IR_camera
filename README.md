# Odroid Go thermal infrared camera

## Introduction

This is a simple IR (infrared) thermal camera project for the Odroid Go handheld ESP32 system. It allows saving of data to an SD card as well as having a basic Bluetooth interface to wirelessly get data off the camera to a computer, tablet or mobile phone. It's based on the MLX90640 32x24 pixel infrared thermal array modules that you can get relatively inexpensively many laces online. Below is a photo of it in action.

![The thermal camera in action](https://github.com/drandrewthomas/Odroid_Go_thermal_IR_camera/blob/master/Photos/cupheat.jpg)

## Using the Arduino code

There are two ways to use the thermal camera code. The really simple way is to go to the 'FW file' folder and copy the 'goircam.fw' file to the firmware folder on your Odroid Go SD card. Then when you turn the Go on with the B button held down you should get an option to load the camera firmware. Obviously you'll need the camera built and attached for the firmware to run.

The more advanced way is for people who want to use the Arduino IDE to edit or build the firmware. To do that just use the files in the 'Arduino code' folder as you would do for code in any other Arduino project you have. All of the details are available on the Odroid Go website for setting up the Arduino IDE for the Go, so I won't cover it here.

If you're using the Arduino IDE method and want to create a firmware file, as described on the Go site, the graphics for use with the MKFW utility are included in the 'Graphics' folder.

## Building the thermal camera module

Building the module is very simple, as it uses just the MLX90640 module, with wires for ground, VCC, SCL and SDA (so just power and i2c). According to the Go wiki the header pinout is as follows, so you can wire the module easily even just using some wires and a breadboard.

| Header Pin | Function |
| ---------- |:--------:|
| 1          | GND      |
| 4          | SDA      |
| 5          | SCL      |
| 6          | VCC      |

Once you've got things wired up and tested, the simplicity of the circuit makes it very simple to solder up some header pins on a piece of veroboard to make a more robust connector for the Odroid Go header sockets. Below is a photo of how I did that. I put the header pins on top of the veroboard and soldered the wires to the track on the back, which meant the wires didn't obstruct the connection, although you could just use a bigger piece of veroboard too.

![A photo of the circuit](https://github.com/drandrewthomas/Odroid_Go_thermal_IR_camera/blob/master/Photos/wiringinside.jpg)

## Making a 3D printed enclosure
