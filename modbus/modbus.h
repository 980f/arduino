#pragma once

/**
  * This is not a generic implementation, commands are implemented/added only as they are used.
  * Note: It indirectly enforces a response delay (when a slave) of 1.750ms (waits for a frame timeout before responding), siemens PLC's may need even more than that.  *
  */

#include "eztypes.h"
enum ModErrorCode {
  mb_OK = 0,
  mb_ILLEGAL_FUNCTION, //01 ILLEGAL FUNCTION The function code received in the query is not an allowable action for this device
  mb_ILLEGAL_DATA_ADDRESS, //start address bad, or start+length bad.
  mb_ILLEGAL_DATA_VALUE, //A value contained in the query data field is not an allowable value for this request (not to be used for out of bounds datum, just for illegal format).
  mb_SLAVE_DEVICE_FAILURE, //this device screwed up while trying to service the valid request.
  mb_ACKNOWLEDGE, //command received, but implied action hasn't been completed. see "Poll Program Complete"
  mb_SLAVE_DEVICE_BUSY, //valid command, but can't deal wtih it now.
  mb_unused7,
  mb_MEMORY_PARITY_ERROR, //stored data block has an error
  mb_unused9,
  //nexOL pretends to be a gateway to its heads:
  mb_GATEWAY_PATH_NOT_AVAILABLE, //can't figure out how to get there from here
  mb_GATEWAY_TARGET_DEVICE_NOT_RESPONDING //no response from device, usually means device doesn't exist.
};

/**
  * values for function code slot of PDU.
  */
enum ModFunction {
  mbfUnknown = 0, //used internally to indicate no PDU present
  mbfReadCoils, //read nominal discrete controls (read-write), there is no 'read one coil' command.
  mbfReadInputs, //read nominal discrete sensors (read-only)
  mbfReadHoldingRegisters, //for read-write items.
  mbfReadInputRegisters, //typically for read-only items
  mbfWriteCoil, //write one nominal discrete control
  mbfWriteRegister, //write one 16 bit location
  mbfReadExceptionStatus, //
  mbfDiagnostic, //
  mbf9,
  mbf10,
  mbfReadCommEventCounter, //
  mbfReadCommEventLog,
  mbf13, mbf14,
  mbfWriteMultipleCoils,
  mbfWriteMultipleRegisters, //16/0x10
  mbfReadSlaveIdinfo, //
  mbf18, mbf19,
  mbfReadFileRecord,
  mbfWriteFileRecord,
  mbfMaskedWrite,
  mbfWriteReadRegisters,
  mbfReadFifo
};

/**@returns whether @param functionCode implies data going to slave */
bool isWrite(ModFunction functionCode);
/**@returns whether @param functionCode implies data going to master */
bool isRead(ModFunction functionCode);
/**@returns whether @param functionCode implies data is input not a coil */
bool isReadOnly(ModFunction functionCode);
/**@returns whether @param functionCode implies data is coil else is register */
bool isCoiled(ModFunction functionCode);
/**@returns whether @param functionCode has its error flag set */
inline bool reportsError(ModFunction functionCode){
  return bit(functionCode, 7);
}


/**
  * non character codes passed to parser where a character is expected, kinda like C's -1 for end-of-file.
  */
enum ModEvent {
  mbeLineError = -99, //uart error, packet is corrupt, slave should not respond, master can't trust the packet.
  mbeInternalError, //software is so confused, good luck recovering
  mbeCharTimedout, //inter character timeout occured
  mbeFrameTimedout, //generally a good thing! bus is free for next transmission
  mbeCommandTimedout, //not a good thing but happens often during development
};


/** accessor for a block of registers, doesn't contain their values */
class ModbusBlock {
public:
  //isCoils might be worth protecting.
  bool isCoils;
  u8 luno;   //7th byte of TCP header, address byte of rtu. Note: TCP typically 0xFF, other values are typical for gateways. We will pretend to be a gateway and use FF for system entities and other values for the per head stuff.
  bool isRO;  //true for inputs, false for coils.
  u16 address;
  u16 length; //quantity of registers or coils
public:
  /** default args yield non-functional object */
  ModbusBlock(bool isCoils = false, int luno = 0, bool isRO = false, unsigned addr = 0, unsigned length = 0);
protected:
  //so that we can't construct coils from registers and vice-versa
  ModbusBlock(const ModbusBlock&other);
public:
  /** copy content, @returns this */
  ModbusBlock& operator =(const ModbusBlock&rhs);
  /** includes 'type' check for coil vs register, so ok to do in base class.*/
  bool operator ==(const ModbusBlock&rhs) const;
  void forget(void); //for internal diagnostics, added while having trouble with processing packets twice on IFT while incrementally parsing.
  /** for sorting for generic logger/status display */
  bool operator <(const ModbusBlock&rhs) const;
  /** byte length field of packet, a.k.a payload size */
  u8 lengthCode() const;
  /** for issuing a command */
  ModFunction functionCode(bool forWrite) const;
  /** @returns whether this block contains address @param which*/
  bool contains(u16 which) const;
  /** @returns whether this block contains address @param which, and if so replaces 'which' with the offset within the group, dividing as well according to @param sizeofOne  */
  bool handles(u16&which, unsigned sizeofOne = 1) const;
  /** @returns whether all of the registers of the block are in this block, optionally ignoring the luno field according to  @param ignoreLuno*/
  bool contains(const ModbusBlock&block, bool ignoreLuno = false) const;
  /** @returns a value suitable for the start address of a block following this one.*/
  u16 end() const;
  /** @returns whether block is a single item at @param address */
  bool isSingle(u16 address) const;

  /** generate/scan array index for/over a piece of a modbus block */
  class Indexer {
    const ModbusBlock&block;
    u16 offset;
    /** how many modbus addresses are spanned by each item */
    int sizeofItem;
  public:
    Indexer(const ModbusBlock&block,int sizeofItem = 1);
    /** @returns whether there is another item in the block given @param number of entities (coil or register) in the block.*/
    bool hasNext();
    /** @returns modbus address of next item, call before you call next(). */
    u16 address();
    /** @returns index of item, post increments by @see sizeofItem */
    int next();
  };

  /** @returns quantity of items between this block's start and @param parentBlock's start.
usage:  this block is a modbus command, the registers therein are a subset of the parentBlock which is a description of the responding slave */
  int bias(const ModbusBlock&parentBlock, int sizeofItem);
};

/** address goes up while length goes down.
  * this class is @deprecated, @see Indexer class inside ModbusBlock
  */
struct ModbusPointer {
  u16 address;
  u16 length; //quantity of registers or coils
  /** for use as iterator: @returns whether length is >0 */
  bool hasNext();
  /** for use as iterator: @returns present address, post increments it and decrements length */
  u16 next();
  /** for multiword object checks whether rest is present, bumping the pointer and counter */
  bool hasMore();
  /** create pointer into @param given block */
  ModbusPointer(const ModbusBlock&block);
};

/** for legacy */
struct RegisterBlock: public ModbusBlock {
  RegisterBlock(int luno = 0, bool isRO = false, int addr = 0, int length = 0);
  RegisterBlock(const RegisterBlock&other);
  RegisterBlock(const ModbusBlock&other);
  /** @returns whether block is a single 32 bit entity at given address. */
  bool isSingle32(u16 address);

};

/** marker class */
struct CoilBlock: public ModbusBlock {
  CoilBlock(int luno = -1, bool isRO = false, unsigned addr = 0, unsigned length = 0);
  CoilBlock(const CoilBlock&other);
};

