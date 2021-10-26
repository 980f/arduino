# 980f's arduino utilties

This is a curated import from other 980f git repos: ezcpp, and dro/arduino.

It uses 980f/ezcpp as is, which the developer clones below this repo like is common for submodules, but doesn't actually declare it to be a submodule.

IE if you find missing files then:
cd <your arduino install>/libraries/980f
git clone https://github.com/980f/ezcpp.git


The above line is in "onclone.sh".

When this gets much larger then some parts will be pushed into subdirectories such as widgets for things like clockhand class and dev for things like the EDS Infrared device.
The I2C I/O expanders will stay at the higher level, as will the most common peripherals such as L298 and L293 motor drivers.


