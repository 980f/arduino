#ifndef TIMERSERVICE_H
#define TIMERSERVICE_H

/**
  To use this service one includes this file in your source, usually main, and polledtimer.cpp to your build.
  polledtimer.cpp has an implementation for PolledTimerServer.

  Each polledtimer object you statically declare is put into a list via linker magic. 
  PolledTimerServer expects to be called for each tick of the system timer, and it does a divide down for each polledtimer calling each timer's action code when the divide goes to zero.

*/
#include "polledtimer.h"
extern "C" int sysTickHook(){
  PolledTimerServer();//all of our millisecond stuff hangs off of here.
  return false; //allowed standard code to run
}
#include "tableofpointers.h"

#endif
