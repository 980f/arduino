#pragma once //(C) 2019 Andy Heilveil, github/980f

//this is a code fragment. You will have to define Console for the output.

void scanI2C() {
  Console("\nScanning for I2C devices:");
  for (byte address = 16; address < 123; address++ )  {
    Wire.beginTransmission(address);
    auto error = Wire.endTransmission();
    switch (error) {
      case 0://success
        Console("\nI2C device found at address ", address);
        break;
      case 1://data too long to fit in transmit buffer
        break;
      case 2://received NACK on transmit of address
        break;
      case 3://received NACK on transmit of data
        break;
      case 4://other error
        Console("\nUnknown error for address ", address);
        break;
      default:
        Console(".");
        break;
    }
  }
  Console("\nScanning for I2C devices is done.");
}
