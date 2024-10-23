#include "modbustcpslave.h"
#include "cheapTricks.h"
#include "logger.h"

ModbusTcpSlave::ModbusTcpSlave(int fd, u32 ipv4, ModbusSlave *agent, bool sharedAgent):TcpSocket(fd,ipv4),
  agent(agent),
  sharedAgent(sharedAgent),
  idleDetector(5000,MyHandler(ModbusTcpSlave::goneQuiet)){
  agent->parser.start(true);//
  idleDetector.retrigger();
}

ModbusTcpSlave::~ModbusTcpSlave(){
  if(!sharedAgent){
    delete agent;
  }
}

void ModbusTcpSlave::reader(ByteScanner &raw){
  if(agent){//COA
    while(raw.hasNext()){
      agent->incrementalReception(raw.next());//which calls processReceived which calls either agent receive or agent send. So, all we really need to do here is route bytes.
      if(flagged(agent->replyPending)){
        idleDetector.retrigger();
        writeInterest();
        raw.dump();//needed for debug-after-breakpoint, should do nothing under normal operating conditions
      }
    }
  }
}

bool ModbusTcpSlave::writer(ByteScanner &raw){
  if(agent){
    raw.grab(agent->lastSent.reader);
    agent->parser.start(true);//prepare for next incoming
    return true;
  }
  return false;
}

void ModbusTcpSlave::goneQuiet(){
  dbg("service gone quiet for port: %d, fd: %d",this->connectArgs.port,sock.fd);
  //todo:2 can we get remote ip?
  notifyConnected(false);
}
