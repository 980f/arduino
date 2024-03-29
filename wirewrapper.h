#pragma once

#include "Wire.h" //TwoWire class

/** condensed use of I2C. */


/** symbols for value returned from endTransmission:
*/
enum WireError : uint8_t {
  None = 0,
  BufferOverflow,
  NackedAddress,
  NackedData,
  Other
};

/** remembers your base address and which bus you are using, and keeps the last error handy. */
class WireWrapper {

    static unsigned bus_kHz;//todo: will need to be an array coindexed with bus selection, or learn how to read it back from the TwoWire object.
    unsigned kHz;
    //internal shared code
    WireError writeBlock(const uint8_t *bites, unsigned qty, bool reversed);
  public://for debug
    const uint8_t base;
    TwoWire &bus;
    WireError lastOp;
    enum { StandardRate=100, FastMode=400, FastModePlus=3400}; //FM+ not yet testable, need an MCU that supports it.
  public:
    /** 7 bit address (arduino convention), kHz (e.g. 100 not 100000), for due etc 0 based selection of which */
    WireWrapper( uint8_t addr, unsigned which = 0, unsigned kHz = StandardRate): base(addr),
#if defined(ARDUINO_SAM_DUE)
      bus(which ? Wire1: Wire) //so far only two are supported.
#else
      bus(Wire) //only one supported at present.
#endif
      , kHz(kHz)
    {
      //#done
    }
    /** someone needs to call begin on the bus, preferable just one entity.*/
    void begin() {
      bus.setClock(kHz * 1000);
      bus.begin();
    }

    /** send another byte, call Start sometime before you start calling this */
    void emit(uint8_t bite) {
      bus.write(bite);
    }

    /** send another byte, call Start sometime before you start calling this */
    void emit(const uint8_t *bites, unsigned qty) {
      bus.write(bites, qty);
    }

    /** take control of the I2C bus */
    void Start();

    /** first phase of common device pattern of a register select before an operation */
    void Start(uint8_t addr) {
      Start();
      emit(addr);
    }

    /** end an I2C operation, @param stopit should be false if it is the first part of a repeated start situation */

    WireError End(bool stopit = true) {
      return lastOp = WireError(bus.endTransmission(stopit));
    }

    /** @returns whether device is present, ACK's its base address.*/
    bool isPresent() {
      Start();
      return End() == WireError::None;
    }

    /** reads a block into internal buffer */
    unsigned Read(unsigned numBytes) {
      return bus.requestFrom(base, numBytes);
    }

    /** send a block of data, with @param reversed determining byte order. default is what is natural for your processor, so you should probably never use the default! */
    WireError Write(const uint8_t *peeker, unsigned numBytes, bool reversed = false) ;

    /** send a block of data to an 8 bit subsystem of your device.*/
    WireError Write(uint8_t selector, const uint8_t *peeker, unsigned numBytes, bool reversed = false);

    /** writes a select then reads a block */
    unsigned ReadFrom(uint8_t selector, unsigned numBytes) ;

    /** writes a select then reads a block into a buffer IFFI the whole block was read, else partial read data is still available */
    bool ReadFrom(uint8_t selector, unsigned numBytes, uint8_t *peeker, bool reversed = false);

    /** @returns amount of read data in Wire buffer*/
    unsigned available() {
      return bus.available();
    }

    /** @returns next read datum from Wire buffer*/
    uint8_t next() {
      return bus.read();
    }

  public: //now for conveniences
    bool seemsOk() {
      return lastOp == WireError::None;
    }

    /** modify a byte at an address */
    uint8_t update(uint8_t addr, uint8_t ones, uint8_t zeroes) {
      uint8_t was;
      if (ReadFrom(addr, 1, &was)) {
        Start(addr);
        emit((was | ones) & ~zeroes); //#parens required to ensure order of operations. without them the ones&~zeroes combined first, then were or'd into mode, preventing us from clearing bits.
        End();
        return was;
      }
      return ~0;
    }

    /** for simple single chunk devices */
    template <typename Scalar> WireError send(Scalar datum) {
      return Write(&datum, sizeof(Scalar));
    }

    /** for simple single chunk devices. If not all bytes are read then datum is unchanged and this returns false */
    template <typename Scalar> bool fetch(Scalar &datum, bool reversed = false) {
      if (Read(sizeof(Scalar)) != sizeof(Scalar)) {
        return false;
      }

      if (sizeof(Scalar) == 1) {//expedite common case, reversed is moot
        datum = next();
        return true;
      }
      uint8_t *peeker = &datum;
      for (unsigned bc = sizeof(Scalar); bc-- > 0;) {
        uint8_t bite = next();
        if (reversed) {
          peeker[bc] = bite;
        } else {
          *peeker++ = bite;
        }
      }
      return true;
    }

};


/** make direct mapped I2C chunk look like a simple variable.
    Known base for PCF8574 and PCF8575  */
template <typename Scalar> class WIredThing : public WireWrapper {
  public:
    WIredThing ( uint8_t addr, unsigned which = 0, unsigned kHz = 100): WireWrapper(addr, which, kHz) {

    }
    //read
    operator Scalar() {
      Scalar red;
      if (fetch(&red)) {
        return red;
      } else {
        return 0;//todo: what is best default for failure? zeroes?
      }
    }

    Scalar operator =(Scalar output) {
      send(output);
      return output;
    }

};
////////////////////////////////////////////////////////////////////////////////

/** a register within a multi register I2C device made to look like a simple variable.
    presently only handles devices with single byte register addresses.
  And yes, it is capitalized WEird because I consistently typoed it.*/
template <typename Scalar> class WIred {
    enum {numBytes = sizeof(Scalar)};
    WireWrapper &ww;
    const uint8_t selector;
    /** I2C might have different endianess than platform. The default arg below should be a platform derived value */
    bool bigendian;//todo: move this onto ww if we see that no devices have per-register endianness (I've seen this with some non-I2C devices)

  public:
    /** each time we read or write we update this value, handy for sequential bit flipping, but prevents consting the class instances. 
    If you write to it then the modify() function will probably not do what you expect */
    Scalar cached;
    WireError lastOp; //each register has its own status separate from the device as a whole.
  public:
    /** @param ww is the device address container. @param selector is one of 256 subsystems within the device. @param bigendian controls the order in which the bytes of a block are moved to and from the device. */
    WIred( WireWrapper &ww, uint8_t selector, bool bigendian = false): ww(ww), selector(selector), bigendian(bigendian) {}

    /**write to device register */
    void operator=(const Scalar &value) {
      lastOp = ww.Write(selector, reinterpret_cast<const uint8_t*>(&value), numBytes, bigendian);
      if (lastOp == WireError::None) {
        cached = value;
      }
    }

    /** @returns value read from device, check lastOp to see if it happened.  */
    Scalar fetch() {
      if (ww.ReadFrom(selector, numBytes, reinterpret_cast<uint8_t*>(&cached), bigendian)) {
        lastOp = WireError::None;
      }
      return cached;
    }

    /** makes a device read appear to be a simple variable access */
    operator Scalar() {
      return fetch();
    }

    /** set @param ones, clear @param zeroes, @returns value before these changes */
    Scalar modify(Scalar ones, Scalar zeroes) {
      Scalar was = cached;
      operator =((cached | ones) & ~zeroes);
      return was;
    }

};
