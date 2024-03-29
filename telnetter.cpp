#include "telnetter.h"

#include "chainprinter.h"
extern ChainPrinter dbg;//todo: bring in logging system concepts from safely repo

#include "strz.h"

unsigned Credentials::save(unsigned offset ) { //todo: symbols or static allocator.
  return strzsave(password, sizeof(password), strzsave(ssid, sizeof(ssid), offset));
}

unsigned Credentials::load(unsigned offset ) {
  return strzload(password, sizeof(password), strzload(ssid, sizeof(ssid), offset));
}

/** safe set of ssid, clips and ensures a trailing nul */
Credentials& Credentials::setID(const char *s) {
  strzcpy(ssid, sizeof(ssid), s);
  return *this;
}

/**safe set of password, clips and ensures a trailing nul */
Credentials& Credentials::setPWD(const char *s) {
  strzcpy(password, sizeof(password), s);
  return *this;
}

bool Telnetter::Verbose=false;  //extra spew

Telnetter::Telnetter(TelnetActor *actor, uint16_t teleport):
  teleport(teleport),
  testRate(500), //500=2 Hz
  server(teleport),
  actor(actor) {
  //#done
}

/** you may call this in setup, or at some time later if you have some other means of getting credentials */
bool Telnetter::begin(const Credentials &acred) {
  cred = &acred;
  beTrying = cred != nullptr;
  return tryConnect();//early first attempt, not strictly needed.
}


/** call this frequently from loop()
    calls back with chunks from clients.
    Feeds merged wifi data from all clients to the given stream.
*/
bool Telnetter::serve( ) {//todo: use functional that matches Stream::write(uint8*,unsigned)
  if (amConnected) {
    if (server.hasClient()) {//true when a client has requested connection
      if (!findSlot()) {//no free/disconnected spot so reject
        server.available().stop();//?forceful reject to client?
        dbg("\nIncoming connection rejected, no room for it.\n");
      }
    }
    //check clients for data
    for (unsigned ci = MAX_SRV_CLIENTS; ci-- > 0;) {
      auto &aclient = serverClients[ci];
      if (aclient && aclient.connected() && aclient.available()) {
        //todo: emit prefix to allow demuxing of multiple clients
        //get data from the telnet client and push it to the UART
        size_t len = aclient.available();
        uint8_t sbuf[len];//not standard C++! (but most compilers will let you do it)
        aclient.readBytes(sbuf, len);
        if (actor) {
          actor->onInput(sbuf, len, ci);
        }
      }
    }
    return true;
  } else {
    if (amTrying) {
      if (testRate.perCycle()) {
        amConnected = testConnection();
      }
    } else if (beTrying) {
      tryConnect();
    }
    return false;
  }
}

/** sends given data to all clients */
void Telnetter::broadcast(const uint8_t &sbuf, const unsigned len, const unsigned sender) {
  //push UART data to all connected telnet clients
  for (unsigned ci = MAX_SRV_CLIENTS; ci-- > 0;) {
    if (ci == sender) {
      continue;//don't echo, although you can set this to something outside the legal range and then it will.
    }
    auto &aclient = serverClients[ci];
    if (aclient && aclient.connected()) {
      aclient.write(&sbuf, len);
    }
  }
}

/** sends data from given stream to all clients */
void Telnetter::broadcast(Stream & stream) {
  if (size_t len = stream.available()) {
    uint8_t sbuf[len];//not standard C++! (but most compilers will let you do it)
    stream.readBytes(sbuf, len);
    broadcast(*sbuf, len);
  }
}

bool Telnetter::tryConnect() {
  if (cred) {
    WiFi.mode(WIFI_STA);
#ifdef UseESP32
    WiFi.softAPsetHostname(actor ? actor->hostname() : "Voldemort");//' Voldemort' he who shall not be named
#else
    WiFi.hostname(actor ? actor->hostname() : "Voldemort");//' Voldemort' he who shall not be named
#endif
    WiFi.begin(cred->ssid, cred->password);

    dbg("\nDevice Mac ");

#if 1 //def UseESP32
    String mac = WiFi.softAPmacAddress();
    dbg(mac);
#else
    MAC mac;
    WiFi.macAddress(mac);
    for (unsigned mi = 0; mi < WL_MAC_ADDR_LENGTH; mi++) {
      Char c(mac[mi]);
      dbg(':', c.hexNibble(1), c.hexNibble(0));
    }
#endif
    dbg("\nConnecting to AP ", cred->ssid);
    if (Verbose) {
      dbg(" using password ", cred->password);
    }
    amTrying = true;
  } else {
    amTrying = false;
  }
  return amTrying;
}

bool Telnetter::testConnection() {
  wst = WiFi.status();
  dbg("\tAPCon status ", wst);
  if (wst == WL_CONNECTED) {
    server.begin();
    server.setNoDelay(true);

    dbg("\nUse 'telnet ", WiFi.localIP());
    if (teleport != 23) {
      dbg(teleport);
    }
    dbg("' to connect.\n");
    amTrying = false; //trying to close putative timing gap
    return true;
  } else {
    return false;
  }
}

/** see if we have a place to keep client connection info. If so retain it.*/
bool Telnetter::findSlot() {
  for (unsigned ci = MAX_SRV_CLIENTS; ci-- > 0;) {
    auto &aclient = serverClients[ci];
    if (aclient && !aclient.connected()) {//connection died
      aclient.stop();//clean it up.
    }
    //not an else! aclient.stop() may make aclient be false.
    if (!aclient) {//if available
      aclient = server.available();
      dbg("\nNew client: #", ci); //todo: find out how to get client ip, will need to go around wifi class proper to do so.
      if (actor) {
        actor->onConnect(aclient, ci);
      }
      return true;
    }
  }
  return false;
}
