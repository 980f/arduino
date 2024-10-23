#pragma once

#include "modpdu.h"
#include "eztypes.h"
#include "bigendianer.h"
#include "modparser.h"


class ModPDU {
protected:
  ModPDU (u8 *buffer, int bufferSize); //hard coded buffer size to its maximum RTU, still need an explicit constructor to init the writer and reader.

  virtual void preamble(const ModbusBlock &block, bool includeAddress, bool startBlock);
  /** @returns transactionID if positive, negative number of missing bytes if length implies packet is trashed/incomplete */
public:

  enum PacketQuality {
    Empty,
    WellFormed,
    TooShort /*too bad to even checksum*/,
    Corrupt /*all good except bad checksum*/
  };
protected:
  virtual ModPDU::PacketQuality parsePreamble(){
    return WellFormed;//no preamble in mostderivatives
  }
  /** how much of received body hasn't been inspected */
  bool stillHave(int body);
  /** @return sizeof postamble. */
  virtual unsigned postamble(){
    return 0;//default is no postamble.
  }
public:
  /** use is to construct one as a local at need and parse or format a whole block at once.*/
  class CoilPointer {
    public:
    Indexer <u8> &buffer;
    u8 *peek;
    int bitIndex;
    CoilPointer(Indexer <u8> &buffer);
    void operator =(const CoilPointer other);
    /** @returns next bit. user should not be reading from buffer between calls*/
    bool next(int howMany=1);
    /** @returns whether packet is exhausted */
    bool hasNext();
    /** appends a coil to the packet, if there is room */
    void pack(bool coil);
  };

public:
  ModFunction functionCode;

  bool isRequest; //else is response
  /** these will wrap buffer allocated in Derived class. */
  BigEndianer writer;
  BigEndianer reader;
  /**guts for beginRequest and beginResponse*/
  BigEndianer & formatPacket(const ModbusBlock &block, bool forWrite,bool includeAddress,bool startBlock);
  /** master begins formatting a request */
  BigEndianer & beginRequest(const ModbusBlock &block, bool forWrite);
  /** slave begins formatting a response */
  BigEndianer & beginResponse(const ModbusBlock &block, bool forWrite);
  /** slave error response */
  BigEndianer & errorResponse(ModErrorCode error,const ModbusBlock &block, bool forWrite);
protected:
  PacketQuality verifyError(PacketQuality theError);//#a breakpoint
  /** no crc in base class */
  virtual u16 checkCRC(void){ return 0;}

  PacketQuality verifyBlock(bool rollbackMore);
  PacketQuality verifyAfter(unsigned int skip, unsigned int backMore );
public:
  PacketQuality verifyAndParse(ModParser&parser);
  /***/
  PacketQuality verifyStart(bool asRequest);//todo:1 find the code for this, perhaps is the incremental verifier?
  /***/
  PacketQuality verify(bool isRequest);//todo:1 find the code for this

  /**call this after all data is concatenated. It prepares the internal pointers for sending*/
  virtual void issuePacket(bool asRequest);

  inline void issueRequest(void){
    issuePacket(true);
  }

  inline void issueResponse(void){
    issuePacket(false);
  }
  //respond to what is in the packer, referencing the given parser for convenience.
  BigEndianer & respond(const ModParser &parsed);

  bool isComplete; //set by writer, not internally managed.
  /** @returns whether packet affirmatively contains a supported write operation.*/
  bool isWrite(void)const{
    return ::isWrite(functionCode);
  }
  /** response is an error reporting packet*/
  bool reportsError(void)const{
    return ::reportsError(functionCode);
  }
  /** @returns whether incoming is the same as the last outgoing, i.e. we see an echo.*/
  bool verifyEcho(u8 incoming);

  /** push interface, suitable for calling with each byte received from serial port:*/
  void beginReception(void);
  /** accept byte into partial packet, @returns whether there was room for it.*/
  bool accept(u8 incoming);
};

