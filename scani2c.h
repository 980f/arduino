#pragma once //(C) 2019 Andy Heilveil, github/980f

#include <Wire.h> //TwoWire class

/** scan for presence in range given by 7-bit addresses */
void scanI2C(ChainPrinter &Console, unsigned highest = 123, unsigned lowest = 16) {
  Console("\nScanning I2C bus:");
  unsigned tested = 0;
  unsigned found = 0;
  for (byte address = lowest; address < highest; address++) {
    ++tested;
    Wire.beginTransmission(address);
    auto error = Wire.endTransmission();
    switch (error) {
    case 0://success
      ++found;
      Console("\n\tfound ", address);
      break;
    case 1://data too long to fit in transmit buffer
      break;
    case 2://received NACK on transmit of address
      break;
    case 3://received NACK on transmit of data
      break;
    case 4://other error
      Console("\nUnkerr @ ", address);
      break;
    }
  }
  Console("\nFound ", found, " out of ", tested, " tested.");
  Console("\nScanning I2C done.");
}
