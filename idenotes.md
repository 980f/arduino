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
* _ideinstall_/hardware

platform.local.txt in same directory as platform.txt overrides it (loads later)

The one I change the most is the cpp language level such as `-std=gnu++17`

---
## #defines
The #defines for detecting and accommodating board and processor variations that matter to your code are in the _platform.txt_ files.

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
The paths for include files are all over the place. _platform.txt_ has macros for most of them.

Arduino.h  for adafruit samd is found in 
/home/andyh/.arduino15/packages/adafruit/hardware/samd/1.7.5/cores/arduino/Arduino.h
Wire.h:
/home/andyh/.arduino15/packages/adafruit/hardware/samd/1.7.5/libraries/Wire/Wire.h



## boards.txt

ARDUINO_{build.board}


core.a library is built from:


## build hooks
recipe.hooks.core.prebuild.NUMBER.pattern (called before core compilation) 
recipe.hooks.core.postbuild.NUMBER.pattern (called after core compilation) 
recipe.hooks.sketch.prebuild.NUMBER.pattern (called before sketch compilation) 
recipe.hooks.sketch.postbuild.NUMBER.pattern (called after sketch compilation) 
recipe.hooks.libraries.prebuild.NUMBER.pattern (called before libraries compilation) 
recipe.hooks.libraries.postbuild.NUMBER.pattern (called after libraries compilation) 
recipe.hooks.linking.prelink.NUMBER.pattern (called before linking) 
recipe.hooks.linking.postlink.NUMBER.pattern (called after linking) 
recipe.hooks.objcopy.preobjcopy.NUMBER.pattern (called before objcopy recipes execution) 
recipe.hooks.objcopy.postobjcopy.NUMBER.pattern (called after objcopy recipes execution) 
recipe.hooks.savehex.presavehex.NUMBER.pattern (called before savehex recipe execution) 
recipe.hooks.savehex.postsavehex.NUMBER.pattern (called after savehex recipe execution)

To test: echo {includes} > {/}includes.list

## build process
combine .ino and .pde files with folder named one first, the rest alphabetical.
also combine files under _src_ which are not loaded into tabs in the IDE.
files under _data_ are definitely not compiled.
If not present prefix with _#include <Arduino.h>_
generate prototypes
adding #line directives throughout so error messages point to original files.

sketch.json is used by CLI and WEB.

Library search order
core folder {build.core}
variant folder {build.variant}
compiler {runtime.tools._compiler_.path/...}
Then libraries
 specified using the --library option of arduino-cli compile
 under a custom libraries path specified via the --libraries option of arduino-cli compile (in decreasing order of priority when multiple custom paths are defined)
 under the libraries subfolder of the IDE's sketchbook or Arduino CLI's user directory
 bundled with the board platform/core ({runtime.platform.path}/libraries)
 ?? bundled with the referenced board platform/core
 bundled with the Arduino IDE ({runtime.ide.path}/libraries)
 
 Matching Rules
 remove .h from #include pathname
 match name
 match name-master
 match nameMoreText   regexp name*
 match MoreTextname     regexp *name
 match wrappednamewoof  regeco *name*
 

Foo
|_ arduino_secrets.h
|_ Abc.ino
|_ Def.cpp
|_ Def.h
|_ Foo.ino
|_ Ghi.c
|_ Ghi.h
|_ Jkl.h
|_ Jkl.S
|_ sketch.json
|_ data
|  |_ Schematic.pdf
|_ src
   |_ SomeLib
      |_ library.properties
      |_ src
         |_ SomeLib.h
         |_ SomeLib.cpp


Arduino.h
1- Declaration of most built-in functions of Arduino like pinMode(), digitalWrite(), ...etc.
2- Macros of some constants like HIGH, LOW, INPUT, OUTPUT, ..etc.
3- Macro functions for bitwise operations and some other general operations like: min(), rand(), ..etc.
4- Declaration of pin and port mapping arrays(check our micro-blog about mapping arrays in Arduino) in flash memory to make a map between Arduino pin number and the physical number. I.e. mapping pin 13 to the according port and pin in the MCU registers. The value of these arrays can be found here: _hardware\arduino\avr\variants\standard\pins_arduino.h_
main.cpp	Here where the basic structure of Arduino program is declared. Here is the actual program:
int main(void)
{
init();
initVariant();
setup();
for (;;) {
loop(); }
return 0; }


Document Resources:
https://arduino.github.io/arduino-cli/0.20/
