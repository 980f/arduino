#pragma once //(C) 2019 Andy Heilveil, github/980f

//this is a code fragment. You will have to define Console for the output.
#include "Wire.h" //TwoWire class

void scanI2C() {
  Console("\nScanning I2C bus:");
  for (byte address = 16; address < 123; address++ )  {
    Wire.beginTransmission(address);
    auto error = Wire.endTransmission();
    switch (error) {
      case 0://success
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
  Console("\nScanning I2C done.");
}
