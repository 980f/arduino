#pragma once
#include "tcpsocket.h"
#include "modbusagent.h"

/** servicing a connection */
class ModbusTcpSlave : public TcpSocket {
  /** this will be a piece of a NexSystem */
  ModbusSlave *agent;
  bool sharedAgent;
  DeferedExecutor idleDetector;

  /** remote has gone quiet*/
  void goneQuiet();

public:
  ModbusTcpSlave(int fd,u32 ipv4,ModbusSlave *agent,bool sharedAgent=false);
  virtual ~ModbusTcpSlave();
  /** called when some data has arrived. You MUST copy the data, the underlying pointer of @param raw is to a piece of the stack. */
  void reader(ByteScanner&raw);

  /** called when can write, should set ByteScanner to point to data, and return true if should be sent.
  The data YOU point to by modifying @param raw must stay allocated until the next call to writer(). You could poll the TcpSocket to see if it is done with the write, we should probably add a callback for 'transmit buffer empty'.*/
  bool writer(ByteScanner&raw);

};

