#pragma once

#include "modbus.h"

/** this class only implements parsing of commands of interest to us, not the full set.
  it contains shared fields, but doesn't retain the packet nor its payload.
*/
struct ModParser {
  bool isTcp;
  bool asRequest;//we generally run two parsers together with this as the difference.

  enum ModState { //receiver states, named for next byte expected
    mbrCorrupt = 0, //line fault or otherwise unknown state, waiting for interframe timeout
    mbrTransactionId, //tcp header first word
    mbrProtocolId, //tcp header fixed second word
    mbrPduLength,  //tcp header additional length field.
    mbrLuno,
    mbrFunction,
    mbrAddress,
    mbrQty,
    mbrByteCount,
    mbrBody,
    mbrCrc,
    mbrErrorCode,
    mbrComplete
  };

  ModState state;
  //accumulation of bytes to make 16 bit quantity:
  bool expectLow;//which byte is expected
  u8 previous;//first of two bytes
  /** finishes accumulating 16 bit item (concatenates previous and @param incoming) */
  u16 hilo(u8 incoming);
  /** check 16 bit object phase, @returns whether item is complete, and @param word has been updated.*/
  bool lowByte(u16& word, u8 incoming);

  /** diagnostic */
  int bytesProcessed;
public: //TCP fields:
  u16 transactionId;
  u16 tcpLength;
public: //rtu fields
  u16 crcsum;
public: //common fields
  /** single coil write is tracked via functionCode, but its values are stuffed here.*/
  ModbusBlock info;
  ModFunction functionCode;  
  ModErrorCode errorCode;
  /** info.lengthCode() */
  unsigned int byteCount;

  ModParser(bool isTcp);
  void start(bool asRequest);
  /** @returns whether packet is completely present and sane except that rtu crc has not been checked */
  bool isComplete(void);
  /**call this when isComplete triggers a crc check and that fails.*/
  void stop(void);
  /** @returns whether packet is complete, doesn't include checking crc but does include receiving it*/
  bool parse(u8 incoming);
  bool isWrite(void) const;
  /** @returns whether response is an error reporting packet*/
  bool reportsError(void) const;
};

