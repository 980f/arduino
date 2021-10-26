
#pragma once

#include "chainprinter.h"
ChainPrinter dbg(debug); //expects a Stream named debug.  todo: less generic name for this, perhaps a #define with an #ifdef here to disable dbg messages

#include "char.h"  //until chainprinter prints hex. todo: it does, fix the code here.

//max number of simultaneous clients allowed, note: their incoming stuff is mixed together. using 3 as the 8266 supports at most 4 sockets.
#define MAX_SRV_CLIENTS 3

struct Credentials {//todo: use allocation constants from wifi.h for ssid and password max lengths
  char ssid[32];
  char password[64];

  unsigned save(unsigned offset = 0) { //todo: symbols or static allocator.
    return strzsave(password, sizeof(password), strzsave(ssid, sizeof(ssid), offset));
  }

  unsigned load(unsigned offset = 0) {
    return strzload(password, sizeof(password), strzload(ssid, sizeof(ssid), offset));
  }

  /** safe set of ssid, clips and ensures a trailing nul */
  void setID(const char *s) {
    strzcpy(ssid, sizeof(ssid), s);
  }

  /**safe set of password, clips and ensures a trailing nul */
  void setPWD(const char *s) {
    strzcpy(password, sizeof(password), s);
  }

};



struct TelnetActor {
  /**  @param bytes and length are a buffer that you cna't trust to be valid after you return.
       @param sender is the index of a connection */
  virtual void onInput(uint8_t *bytes, unsigned length, unsigned sender) {
    ;//do nothing
  }

  /**  @param aclient is the esp8266 provided object, that you can use e.g. to send a connection response message.
       @param sender is the index of a connection */
  virtual void onConnect(WiFiClient &aclient, unsigned sender) {
    ;//do nothing
  }

  /** @returns a pointer to the TelnetActor's hostname, you must return one stays valid after the return (not an auto variable) */
  virtual const char *hostname() {
    return nullptr;
  }

};


//todo: put into a cpp file
/** a server that rebroadcasts whatever it receives either from a wifi client or the local serial port */
struct Telnetter {
  bool amTrying = false;
  bool amConnected = false;
  bool beTrying = false;

  uint16_t teleport;//retained for diagnostics
  MonoStable testRate; //rate limiter for wifi access port connection checks

  const Credentials *cred = nullptr;

  wl_status_t wst = WL_NO_SHIELD;

  WiFiServer server;
  WiFiClient serverClients[MAX_SRV_CLIENTS];

  TelnetActor *actor;

  Telnetter(TelnetActor *actor, uint16_t teleport = 23): //23 is standard port
    teleport(teleport),
    testRate(500), //500=2 Hz
    server(teleport),
    actor(actor) {
    //#done
  }

  /** you may call this in setup, or at some time later if you have some other means of getting credentials */
  bool begin(const Credentials &acred) {
    cred = &acred;
    beTrying = cred != nullptr;
    return tryConnect();//early first attempt, not strictly needed.
  }


  /** call this frequently from loop()
      calls back with chunks from clients.
      Feeds merged wifi data from all clients to the given stream.
  */
  bool serve( ) {//todo: use functional that matches Stream::write(uint8*,unsigned)
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
  void broadcast(const uint8_t &sbuf, const unsigned len, const unsigned sender = ~0U) {
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
  void broadcast(Stream & stream) {
    if (size_t len = stream.available()) {
      uint8_t sbuf[len];//not standard C++! (but most compilers will let you do it)
      stream.readBytes(sbuf, len);
      broadcast(*sbuf, len);
    }
  }

  bool tryConnect() {
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

  bool testConnection() {
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
  bool findSlot() {
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

} ;

