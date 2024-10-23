#pragma once

#include "eztypes.h" //should rename this arttypes, or wrap it with that
#include "modbusagent.h"

/** usage example of modbus tcp support library */


class EgModbusSlave:public ModbusSlave {
  u16 echoregister;
  u16 hostVersion;
  u16 appSelectCache[4]; //4 = max number of heads + 1, so we don't have to zero index.
  u16 echoCache = 0;
public:
  EgModbusSlave(NexSystem *nex);
  ~EgModbusSlave();
  /** called by processReceived.
   *overrides are expected to read from @param data according to @param block, and @return a code as to whether block is legit */
  ModErrorCode receive(const ModbusBlock &/*block*/, BigEndianer &/*data*/);
  /** called by processReceived.
   *overrides are expected to write on @param data according to @param block, and @return a code as to whether block is legit */
  ModErrorCode send(const ModbusBlock &/*block*/, BigEndianer &/*data*/);
  /** single register read. @param pointer's address gets incremented by one or two according to size of item addressed, and length gets decremented likewise.
The value is put into @param packet, and @returns either mb_OK or an mb_ILLEGAL_DATA_ADDRESS error */
  ModErrorCode getRegisters(ModbusPointer &pointer, BigEndianer &packet);
  /** single register write.
   *@returns either mb_OK or an mb_ILLEGAL_DATA_ADDRESS error
   *@param address gets incremented by one or two according to size of item addressed even if register is read-only.The value is taken from @param packet.*/
  ModErrorCode setRegister(unsigned &address, BigEndianer &packet);
  void headCoilWriter(Head &head, const ModbusBlock &block, BigEndianer &packet);

  ModErrorCode systemCoilSetter(const ModbusBlock &block, BigEndianer &packet);
  /** @returns indicator for some of the coils */
  bool systemCoilGetter(int which);
  int selectedApp(Head &head);
private:
  Head*getHead(int onebased);
  void systemCoilReader(const ModbusBlock &block, BigEndianer &packet);
  void systemInputReader(const ModbusBlock &block, BigEndianer &packet);
};
