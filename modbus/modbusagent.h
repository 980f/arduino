#pragma once

#include "modbustcppacket.h"
#include "stopwatch.h"

//##need a timeout facility for arduino version #include <deferedexecutor.h>
#include <functional>
class DeferedExecutor{//stub for migrating from glib to arduino
public:
    DeferedExecutor(int timeoutMillis, std::function handler);
    /** should stop the timeout */
    void cancel();
    //todo: what is name for starting this?
};

//todo: syntax for converting a memberfunction of 'this' into a std::function for the DeferedExecutor timeout
#define MyHandler(memberfn) this->memberfn

/** processing is very similar between master and slave, and fairly independent of packet */
class ModbusAgent {
protected:
  ModbusAgent(bool asSlave);
  bool asSlave;
public://todo: make protected
  /** for master: argument of pollCommand, retained for debug, and need a buffer to format into.
  * for slave: response */
  ModbusTcpPacket lastSent;

  /** retained for debug, could be local to parse() */
  ModbusTcpPacket received;
  /** Note: in master gets init by command formatter for those fields not echoed by responder.*/
  ModParser parser;
  /** @returns whether @param which is in range of group defined by @param groupbase and @param groupLength, and if so @param which has been modified to be offset */
  static bool inGroup(u16 &which,u16 groupbase,int groupLength);
  /** format the start of a modbus request, have to feed some request info to the response parser due to ill-conceived modbus format.
  @returns a pointer to where write data is to be appended. */
  BigEndianer &beginRequest(ModbusTcpPacket &packet, const ModbusBlock &block, bool forWrite);

  /** overrides @return whether packet was nice */
  virtual bool processReceived()=0;
public://made public for sake of NexModbusTcpSlave
  bool incrementalReception(u8 incoming);
};

/** base class for modbus slave application logic. */
class ModbusSlave: public ModbusAgent {
public:
  ModbusSlave();
  virtual ~ModbusSlave();
protected:
  /** called by processReceived.
   *overrides are expected to read from @param data according to @param block, and @return a code as to whether block is legit */
  virtual ModErrorCode receive(const ModbusBlock &/*block*/, BigEndianer &/*data*/){
    return mb_SLAVE_DEVICE_FAILURE;
  }
  /** called by processReceived.
   *overrides are expected to write on @param data according to @param block, and @return a code as to whether block is legit */
  virtual ModErrorCode send(const ModbusBlock &/*block*/, BigEndianer &/*data*/){
    return mb_SLAVE_DEVICE_FAILURE;
  }
  bool processReceived();
public:
  bool replyPending;
};

/** base class for something that talks to modbus slaves. */
class ModbusMaster: public ModbusAgent {
public:
  /** initial timeout can be overwritten later by altering responseTimer.milliseconds */
  ModbusMaster(int timeoutMillis=1000);
  virtual ~ModbusMaster();
protected:
  /** enforce half-duplex, don't send a command until a response from a previous has been received.
This requires us to implement a response timeout in order to not freeze on a lack of response.
This also means that to communicate with many MB slaves in parallel we need multiple instances of this class. That is not a problem, all those instances can funnel into a single handler application. */
  bool waitingForResponse;
  //## need some timeout utility DeferedExecutor responseTimer;
  virtual void timedOut();
  bool processReceived();
  /** processReceived calls this when the tcp transaction id's mismatch */
  virtual bool tcpError(){
    return false;
  }
  /** processReceived calls this when the response content is bad, parser member has details of the error. */
  virtual void parseError(){}

  /** called by parse, default @returns modbus error code mb_ILLEGAL_DATA_ADDRESS*/
  virtual int setCoil(u16 which,bool on);
  /** called by parse, which uses the return to pick which setter to call:
   0: unknown address
   1: 16 bit
   2: 32 bit integer
   3: 32 bit float */
  virtual int registerType(u16 which);
  /** called by parse, default @returns modbus error code mb_ILLEGAL_DATA_ADDRESS*/
  virtual int setRegister(u16 which,u16 value);
  /** called by parse, default @returns modbus error code mb_ILLEGAL_DATA_ADDRESS*/
  virtual int set32(u16 which,u32 value);
  /** called by parse, default @returns modbus error code mb_ILLEGAL_DATA_ADDRESS*/
  virtual int setFloat(u16 which,float value);
public:
  struct CommStatistics {
    unsigned requests;
    unsigned timeouts;
    unsigned goodResponses;
    unsigned badResponses;
    /** reset all counters */
    void clear();

    CommStatistics();
  } stats;

};

