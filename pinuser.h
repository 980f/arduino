#pragma once

//(C) 2018 by Andy Heilveil, github/980F
//for sharing Arduino and Maple codebases.

#ifdef STMDUINO

using PinNumberType=unsigned;//added this while figuring out I had a bad #if, left it in for formalities sake
using PinModeType=WiringPinMode; //maple lib forces this on us.

#else
using PinNumberType=unsigned;//added this while figuring out I had a bad #if, left it in for formalities sake
using PinModeType=unsigned;
#endif

//the only extension we care about in basic pin stuff. Open Drain etc. needs to be explicit.
#ifndef INPUT_PULLDOWN
#define INPUT_PULLDOWN INPUT
#endif


#ifndef ARDUINO
//stubs to test compile on a desktop development system.
void pinMode(PinNumberType pinnum, PinModeType PINMODE);
bool digitalRead(PinNumberType pinnum);
void digitalWrite(PinNumberType pinnum, bool);
#endif

