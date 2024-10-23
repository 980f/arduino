#include "modbustcpserver.h"
#include "tcpsocket.h"
#include "modbustcppacket.h"
#include "modbusagent.h"

ModbusTcpServer::ModbusTcpServer(Factory factory):TcpServer(),
  config(nullptr),
  factory(factory)
{
  //do nothing until configured
}

void ModbusTcpServer::start(const ModbusServerConfig &config){
  this->config=&config;

  dbg("Serving modbus at IP: %1", config.ethIp.resolved.native());
  dbg("Serving modbus at port: %1", config.port.native());
  TcpServer::serveAt(config.port,config.ethIp.resolved);
}

TcpSocket *ModbusTcpServer::spawnClient(int fd,u32 ipv4){
  //# the factory is allowed to return a nullptr to refuse a connection. The connections object does not append such nulls.
  ModbusTcpSlave *spawned = factory(fd,ipv4,connections.quantity());

  if (spawned){
    //connecting onRemoteDisconnect to handle when the client disconnects cleanly.
    //A zombied connection is handled in the modbustcpslave class using an idle detector.
    spawned->whenConnectionChanges(sigc::bind(MyHandler(ModbusTcpServer::onRemoteDisconnect),spawned));
  }else{
    dbg("Looks like we reached our max on connections. Currently we have: %d",connections.quantity());
  }
  //no need to check for nullptr, handled in connections object
  return connections.append(spawned);
}

void ModbusTcpServer::onRemoteDisconnect(bool /*isConnected*/, ModbusTcpSlave * spawned){
  //NB: don't do if(isConnected) here, we are never connected when this is called because the remote just disconnected.
  spawned->disconnect(false);
  connections.remove(spawned);
}

void ModbusTcpServer::disconnectAll(){
  for(int i=connections.quantity(); i--> 0;){
    connections[i]->disconnect(true);
    connections.removeNth(i);
  }
  
}

void ModbusTcpServer::shutdown(bool permanently){
  connections.clear();//the deletes herein will close the sockets.
  if(permanently){
    config=nullptr;
  }
  TcpServer::shutdown(permanently);
}

ChainScanner<ModbusTcpSlave> ModbusTcpServer::list(){
  return ChainScanner<ModbusTcpSlave> (connections);//# these are cheap enough to copy, don't use 'new'
}
