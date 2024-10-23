#include "modbusserverconfig.h"

///////////////////////
ModbusServerConfig::ModbusServerConfig(Storable &node):Stored(node),
  ConnectChild(port),
  ConnectChild(ethIp),
  ConnectChild(numberOfConnections),
  ConnectChild(watchdogTimeout,5000),
  ConnectChild(ipAddress, "0.0.0.0"),
  ConnectChild(staticIP, false),
  ConnectChild(netmask, "255.255.255.0"),
  ConnectChild(gateway, "192.168.0.1"){
  //#nada
}
