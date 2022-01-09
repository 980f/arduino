# Arduino IDE build tweaking

---
##libraries exist in 3 locations:


* _sketch_/libraries
* _~/.arduino15_/libraries
* _ideinstall_/hardware/arduino/_processorfamily_/libraries

---

## compiler flags are in platform.txt which is in:
* _~/.arduino15_/packages/_packagevendor_/hardware/_processorfamily_/_versionoftool_/
* _ideinstall_/hardware/arduino/_processorfamily_/


The one I change the most is the cpp language level such as `-std=gnu++17`

---
## #defines
The #defines for detecting and accommodating board and processor variations that matter to your code are in the platform.txt files.

For Adafruit SAMD development they are:
  - F_CPU=48000000L
  - CRYSTALLESS
  - ARDUINO_ARCH_SAMD
  - ARDUINO_SAMD_ADAFRUIT
  - ARDUINO_SAMD_ZERO
  - ARM_MATH_CM0PLUS

* Circuit Playground Express
  - ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS
  - ADAFRUIT_CIRCUITPLAYGROUND_M0
  - __SAMD21G18A__


* QTPy
  - ARDUINO_QTPY_M0
  - ADAFRUIT_QTPY_M0
  - __SAMD21E18A__

For the USB product info you can stick your own id info in:
  - USB_MANUFACTURER=\"Adafruit\"
  - USB_PRODUCT=\"QT Py M0\" 
  
  
## #include paths
The paths for include files are all over the place. platform.txt has macros for most of them.

Arduino.h  for adafruit samd is found in 
/home/andyh/.arduino15/packages/adafruit/hardware/samd/1.7.5/cores/arduino/Arduino.h
Wire.h:
/home/andyh/.arduino15/packages/adafruit/hardware/samd/1.7.5/libraries/Wire/Wire.h

