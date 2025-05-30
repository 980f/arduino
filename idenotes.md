# Arduino IDE build tweaking

---
##libraries exist in 3 locations:


* _sketch_/libraries
* _~/.arduino15_/libraries
* _ideinstall_/hardware/arduino/_processorfamily_/libraries

---

## compiler flags are in `platform.txt` which is in:
* _~/.arduino15_/packages/_packagevendor_/hardware/_processorfamily_/_versionoftool_/
* _ideinstall_/hardware/arduino/_processorfamily_/
* _ideinstall_/hardware

`platform.local.txt` in same directory as `platform.txt` overrides it (loads later)
hint: put your `platform.local.txt` in your sketchbook and a link beside the `platform.txt` to guard against losing yours in an upgrade or reinstall.

The one I change the most is the cpp language level such as `-std=gnu++17`

---
## #defines
The #defines for detecting and accommodating board and processor variations that matter to your code are in the `platform.txt` files.

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

For the Adafruit (perhaps others?) USB product info you can stick your own id info in:
  - USB_MANUFACTURER=\"Adafruit\"
  - USB_PRODUCT=\"QT Py M0\" 
  
  
## #include paths
The paths for include files are all over the place. `platform.txt` has macros for most of them.

Arduino.h  for adafruit samd is found in 
`/home/andyh/.arduino15/packages/adafruit/hardware/samd/1.7.5/cores/arduino/Arduino.h`
Wire.h:
`/home/andyh/.arduino15/packages/adafruit/hardware/samd/1.7.5/libraries/Wire/Wire.h`



## boards.txt

ARDUINO_{build.board}
...

???core.a library is built from:


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
Combine .ino and .pde files with folder named one first, the rest alphabetical.
Also combine files under `src` which are not loaded into tabs in the IDE.
files under `data` are definitely not compiled.
If not present prefix with `#include <Arduino.h>` which is found in the core folder for the selected board.
Generate prototypes, an imperfect process which you can fix by putting in your own prototypes as needed.
Add #line directives throughout so error messages point to original files.

`sketch.json` is used by CLI and WEB for ???

### Library search order
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
 match wrappednamewoof  regexp *name*
 
 First the library name from its properties file is run through the above, then the folder name is run through the same.
 The next level of priority is based on the containing path of the library folder
 (explicit listing in cli)
 sketchbook/libraries
 {runtime.platform.path}/libraries
 ?referenced boad platform/core
 {runtime.ide.path}/libraries
 
 In addition to the priority search the library is checked for applicability to the processor via _architectures_ field in _library.properties_ . If that properties file does not have an architectures property then architectures='*' is presumed.

## sketch layout
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

## Arduino.h
1- Declaration of most built-in functions of Arduino like pinMode(), digitalWrite(), ...etc.
2- Macros of some constants like HIGH, LOW, INPUT, OUTPUT, ..etc.
3- Macro functions for bitwise operations and some other general operations like: min(), rand(), ..etc.
4- Declaration of pin and port mapping arrays in code memory to make a map between Arduino pin number and the physical number. I.e. mapping pin 13 to the according port and pin in the MCU registers. The value of these arrays can be found here: _hardware\arduino\avr\variants\standard\pins_arduino.h_
main.cpp	Here is where the basic structure of Arduino program is declared. Here is the actual program:
int main(void) {
	init();
	initVariant();
	setup();
	for (;;) { //in FreeRTOS system there is a yield or similar here, loop isn't hammered quite as hard as in most mcu's.
		loop(); 
	}
	return 0; 
}


## Zip library 
libname
|_ library.properties
|_ keywords.txt
|_ src
		|_ yourmodule.h
		|_ yourmodule.cpp
		|_ other .h and .cpp or .c as needed.
|_ examples
		|_ sketchname
				|_ anexample.ino
|_ extras
		|_ any other files such as usage notes.


## plaforms
Platform folders are either in the users directory (~/.arduino15) or the installation directory. In each folder are at least:
* _platform.txt_
    - compiler
    - build paameters
    - tools definitions
* _boards.txt_
* _programmers.txt_
    - download tooling
    
OS specific variations are indicated by .linux, .windows, .macosx suffixes on the property name.
{runtime.platform.path} is that of the board platform folder (boards.txt)
{runtime.hardware.path} the parent of the platform path.
{runtime.ide.path} path to the arduino executables
{runtime.ide.version}
{name} is the platform vendor name
{_id} is the board ID
{build.fqbn} "fully qualified board name" colon separate fields, includes the options set in the board selection menu.


Document Resources:
https://arduino.github.io/arduino-cli/0.20/
https://arduino.github.io/arduino-cli/0.19/platform-specification/
