# Arduino IDE build tweaking

##libraries exist in 3 locations:

* _sketch_/libraries
* _~/.arduino15_/libraries
* _ideinstall_/hardware/arduino/_processorfamily_/libraries

## compiler flags are in platform.txt which is in:
* _~/.arduino15_/packages/_packagevendor_/hardware/_processorfamily_/_versionoftool_/
* _ideinstall_/hardware/arduino/_processorfamily_/


The one I change the most is the cpp language level such as `-std=gnu++17`

