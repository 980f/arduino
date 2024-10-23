#pragma once

#include "modpdu.h"

/** RTU packet. For legacy sake we are leaving it wiht a generic name */
class ModPacket:public ModPDU {
private:
  void addCrc(void);//adds crc at end of where writer points, writer points past crc.
  u16 checkCRC(void);//reader should be pointing to CRC, will point past it after this returns
protected:
  /** size of postamble */
  unsigned postamble(){return 2;}
public:
  ModPacket (void); //hard coded buffer size to its maximum RTU, still need an explicit constructor to init the writer and reader.
  u8 buffer[256];

  /** @see ModPDU::issuePacket, this appends crc and then calls that. */
  void issuePacket(bool asRequest);

};

