#pragma once //(C) 2019 Andy Heilveil, github/980f

#include <Wire.h> //TwoWire class

/** scan for presence in range given by 7-bit addresses */
void scanI2C(ChainPrinter &Console, unsigned highest = 123, unsigned lowest = 16) {
  auto x=Console.stackFeeder(true);
  Console("\nScanning I2C bus:");
  Wire.begin();
  Wire.setClock(100000);//use slowest rate until we know what is present.
  unsigned tested = 0;
  unsigned found = 0;
  for (byte address = lowest; address < highest; address++) {
    ++tested;
    Wire.beginTransmission(address);
    auto error = Wire.endTransmission();
    switch (error) {
    case 0://success
      ++found;
      Console("\tfound ", address, " 7bit:0x",HEXLY(address), " 8bit:0x",HEXLY(address<<1));
      break;
    case 1://data too long to fit in transmit buffer
      break;
    case 2://received NACK on transmit of address
      break;
    case 3://received NACK on transmit of data
      break;
    case 4://other error
      Console("Unkerr @ ", address);
      break;
    }
  }
  Console("Found ", found, " out of ", tested, " tested.");
  Console("Scanning I2C done.");
}
