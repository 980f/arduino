#pragma once
#include "tcpserver.h"
#include "modbusserverconfig.h"
#include "chain.h"
#include "modbustcpslave.h"

class ModbusTcpServer: public TcpServer{
  Chain<ModbusTcpSlave> connections;//keep list of connections as we will eventually have access controls that need this.
  const ModbusServerConfig *config;
  /** should return a new'd object as this class will delete it when done with it.
@param fd is an open file descriptor for the socket,
@param ipv4 is the remote address (great for ACL's),
@param which is how many already exist */
  typedef sigc::slot<ModbusTcpSlave */*newone*/,int /*fd*/,u32 /*ipv4*/,int /*which*/> Factory;
  /** used by spawnClient */
  Factory factory;
public:
  ModbusTcpServer(Factory factory);
  /** @param config MUST persist as long as this server does!*/
  void start(const ModbusServerConfig &config);
  TcpSocket *spawnClient(int fd, u32 ipv4);
  /** kill all sockets, quit serving, if @param permanently then also forgets the configuration*/
  void shutdown(bool permanently=true);
  /** @returns and iteration like thing over the connections.
   * Added for gui display of existing connections */
  ChainScanner<ModbusTcpSlave> list();
  void onRemoteDisconnect(bool isConnected, ModbusTcpSlave *spawned);
  void disconnectAll();
};
