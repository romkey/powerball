# Powerball

Powerball is a small gadget which:
- listens to the serial out of a controller and remotely log messages from it, including crash information
- listens for a heartbeat and automatically reset the controller after missing too many heartbeats
- monitors and remotely records voltage and current used by the controller to help look for power issues

Powerball is intended to help debug Arduino and other embedded controller projects. It's helpful for debugging projects which aren't convenient to leave hooked up to a computer all the time in order to capture their output. If you have a project that's crashing a lot, Powerball can help you figure out whether it's an electrical issue (is the current too high? Is the voltage too low?) or a software issue. It can even capture 

Powerball is based on Furball, test hardware for HomeBus, which is still in early development.

The name 'Furball' is an homage to Dave Mills' ["Fuzzball"](https://en.wikipedia.org/wiki/Fuzzball_router), one of the first routers on the nascent Internet.

Powerball logs console out to a syslog server. I recommend the free tier of [Papertrailapp](https://papertrailapp.com/).

Powerball logs voltage and current levels to an MQTT server. You can use something like http://io.adafruit.com (which limits you to 30 samples per minute on the free plan and 60 samples per minute on the paid plan, or you can use the [Powerball Server](https://github.com/romkey/powerball-server), which will run under Heroku's free plan.

## Configuration

1. Copy `src/config.h-example` to `src/config.h` and edit the new file.


## Current Status



## Hardware

Powerball is based on the ESP8266 processor. It doesn't need a lot of processing power or storage, so the ESP8266 is a good, low power choice.

Powerball uses an [INA219](https://i2cdevices.org/devices/ina219) I2C current and voltage sensor.

### Total Cost

If bought through AliExpress, parts cost should run roughly:
- $2.15 - Wemos D1 Mini
- $1.50 - INA219
- $1 - random resistors, capacitors

Total of roughly $4.65 in parts before the circuit board.

## Building The Hardware

I'm still working on a schematic, but the hardware is very simple to build.

Powerball and the target board have independent power supplies. Ground is connected together. +5V  for the target is run through the INA219 so that it can measure voltage and current.

Serial TX from the target is run to D4.

## Software

Powerball's firmware uses the Arduino SDK for ESP8266 and is organized to build using PlatformIO, which runs under Linux, MacOS and Windows.

The software provides a uniform, abstract interface to each device using the `Sensor` class. The `Sensor` class provides some housekeeping functions to help the software poll the devices at the appropriate intervals.

# License

Software and documentation are licensed under the [MIT license](https://romkey.mit-license.org/).

Hardware designs are licensed under [Creative Commons Attribution Share-Alike license](https://creativecommons.org/licenses/by-sa/4.0). 
