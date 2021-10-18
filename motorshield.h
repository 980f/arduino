/** seedstudio V1.1 motor shield 
		L298 based dual H-bridge as independent drives (or paired driver)
*/

#include "pinclass.h"


struct SeeedStudioMotorShield {
	
};

//pinout is not our choice
FourBanger<8, 11, 12, 13> L298;
DuplicateOutput<9, 10,32> motorpower; //pwm OK. These are the ENA and ENB of the L298 and are PWM

