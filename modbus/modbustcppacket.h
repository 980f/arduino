#pragma once
#define MODBUSTCPPACKET_H

#include "modpdu.h"

/** packet manager as well as the packet itself.
*/
class ModbusTcpPacket : public ModPDU {
  u8 buffer[260];
public: //can protect if we implement an id compare.
  /** for writes transaction ID generator, for responses should match some other one. */
  u16 transactionId;
protected:
  /** inserts transaction id and most of the MBAP */
  void preamble(const ModbusBlock &block, bool includeAddress, bool startBlock);
public: //todo:1 shove this into ModPDU and make it protected.
  /** @returns modbus error code (or internal error if negative), suitable for a response if this is a server. */
  ModPDU::PacketQuality parsePreamble();
public:
  ModbusTcpPacket();
  /** remove parsed data from writer keeping entities not yet parsed, useful for dealing with multiple mb packets in one ethernet one.*/
  void slide();
  /** prepare input buffer and parser for new request*/
  void prepareForReception();
};

