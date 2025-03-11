#pragma once

/** MacAddress class, a 48 bit item often rendered to text as 6 8bit numbers
*/
struct MacAddress {
  static const unsigned macSize = 6;
  uint8_t octet[macSize];

  /** allow this object to transparently function like an array of bytes, to mate to code expecting such without editing.*/
  operator uint8_t*() {
    return octet;
  }

  operator const uint8_t*() const {
    return octet;
  }

  /** make this object work like an array of bytes */
  uint8_t & operator[](unsigned index) {
    return octet[index < macSize ? index : 0];//on error feed them the first octet, which should generate interesting bug behavior.
  }

  bool operator ==(const MacAddress &other) {
    for (unsigned i = macSize; i-- > 0;) { //optimal order since often the first 3 octets match (this class was crafted for use with esp32 parts)
      if (octet[i] != other.octet[i]) {
        return false;
      }
    }
    return true;
  }

  bool isBroadcast() {
    for (unsigned i = macSize; i-- > 0;) {
      if (octet[i] != 0xFF) {
        return false;
      }
      return true;
    }
  }

  //usage: MacAddress myMac={0xd0,0x47,0x20,0x00,0x21,0x2D};
  // but also MacAddress broadly={0xFF}; or MacAddress none={0};
  MacAddress(std::initializer_list<uint8_t> list) {
    unsigned index = 0;
    for (auto thing : list) {
      octet[index++] = thing;
      if (index >= macSize) {
        return; //user typed too many octets!
      }
    }
    if (index == 0) { //empty braces
      octet[index++] = 0;//which will generate all zeroes.
    }
    while (index < macSize) { //truncated, copy last written byte to all others.
      octet[index] = octet[index - 1];
      ++index; //if inlined above you are in c++ "undefined behavior" territory.
    }
  }

  //write this MAC address into an array of bytes, which had better have room for 6 bytes.
  void operator>>(uint8_t *raw) const {
    memcpy(raw, octet, macSize);
  }

  /** copy @param rhs to this one*/
  MacAddress& operator=(const MacAddress& rhs) = default;

#ifdef Arduino_h
  /** output standard image to @param output, some Print device such as a Serial port */
  void PrintOn(Print &output) {//todo: implement Printable interface and replace this with that.
    for (unsigned index = 0; index < macSize; ++index) { //order was reversed, I checked Espressif's allocated mac prefixes.
      if(index){
        output.print(':');
      }
      output.print(octet[index], HEX);     
    }
  }
#endif

};
