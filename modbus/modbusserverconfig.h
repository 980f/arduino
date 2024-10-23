#ifndef MODBUSSERVERCONFIG_H
#define MODBUSSERVERCONFIG_H

#include "storable.h"
#include "storednumeric.h"
#include <storedipv4address.h>

class ModbusServerConfig: public Stored {
public:
  ModbusServerConfig(Storable &node);
  StoredInt port;
  StoredIPV4Address ethIp;
  StoredInt numberOfConnections;
  //todo: ACL
  StoredReal watchdogTimeout;
  StoredLabel ipAddress;
  StoredBoolean staticIP;
  StoredLabel netmask;
  StoredLabel gateway;

};

#endif // MODBUSSERVERCONFIG_H
