#pragma once //(C)2019 Andy Heilveil. github/980f

/*
  Microchip MCP23017 16 bit I/O.
  
  There is a very similar part with SPI access instead of I2C, we will eventually extract a base class but first will do the I2C version.

  The addresses can be configured to operate it as two independent bytes, or as 16 bits.
  While that can be dynamically changed that would be confusing.
	Initially only 16 bit mode is supported, 8 bit mode will take some c++ finesse to present as two objects.

	Pins are to be configured.
		output else input
		input can be inverted, and has optional 100k pullup (better than nothing?)
		it can generate an interrupt, on Hi / low or any.
	Its I2C address range is the same as the PCF I/O expanders.
		
  
*/
class MCP23017 {
    WireWrapper &my2c; //not deriving so that we can share the master with other devices on the bus. Duplicate master structures might be ok, but let us not depende upon that until we have more experience.
    const byte myaddress; // 7 bit address

    /** we are only supporting 16 bit mode at present. When we do 8 bit mode we will need two instances of a subclass */
		WIred<u16> gpio;
		//WIred has a cache like the 8574/5 did.    
  public:
    /** there are 3 address jumpers, 8 boards allowed per bus */
    MCP23017 (unsigned which = 0, WireWrapper &master): my2c(master,WireWrapper::FastMode), myaddress(0x20 + (which & 7)) ,gpio(my2c,14,true){
			//#nada
    }
    
    void begin(){
    /** iocon is 7 control bits, but most are related to interrupts and we aren't implementing them yet */
	    WIred<u8> iocon(my2c, 5);
	    iocon=0; //todo: add operands to manipulate bits as we add support for the features.    
    }

    /** configure pins to be managed as input-only  */
    void setInput(unsigned mask,unsigned pullups) {
      //write to iodir register
      WIred<u16>cfg(my2c,0,true);
      cfg=mask;
      //write to pullup register
      WIred<u16>cfg2(my2c,1,true);
      cfg2=pullups;  		
    }
    
    /** interrupt config will come later */

    /** @returns whether device seems to be present */
    bool isPresent() {
      return my2c.isPresent();
    }

		/** send to gpio. @returns argument, not the actual state of the pins */
		u16 operator =(u16 allbits){
			gpio=allbits;
			return allbits;
		}
		
		operator u16() {
		  return gpio;
		}


    friend class MCP23017::Bit;

  public:
    /** make the physical pin look like a boolean */
    class Bit {
        const unsigned which; //0..15
        MCP23017 &group;
      public:
        Bit (MCP23017 &group, unsigned which): which(which), group(group) {
          //don't do anything, so that we can sanely static construct.
        }

        operator bool() {//todo: test
          return ((group.fetch() >> which) & 1) == 1; //compiler will generat an extract bit instruction for cortex parts.
        }

/** @returns argument, not the actual pin*/
        bool operator =(bool bit) {
          group.send(bit ? (group.lastwrote | 1 << bit) : (group.lastwrote & ~(1 << bit))); //no need to use assign ops here, the group tracks the word written.
          return bit;
        }
    }

};
