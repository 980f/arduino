
#pragma once

#include "chainprinter.h"
ChainPrinter dbg(debug);
 
#include "char.h"  //until chainprinter prints hex.

//unsigned hexify(uint8_t charish){
//  unsigned fourchars; //sizeof unsigned
//  Char see(charish);
//  //todo; endinaness
//  fourchars=see.hexnibble(0)+(see.hexnibble(1)<<8);
//  return fourchars;
//}



//max number of simultaneous clients allowed, note: their incoming stuff is mixed together. using 3 as the 8266 supports at most 4 sockets.
#define MAX_SRV_CLIENTS 3

struct Credentials {//todo: use allocation constants from wifi.h
  char ssid[32];
  char password[64];
  
  unsigned save(unsigned offset = 0) { //todo: symbols or static allocator.
    return strzsave(password, sizeof(password), strzsave(ssid, sizeof(ssid), offset));
  }

  unsigned load(unsigned offset = 0) {
    return strzload(password, sizeof(password), strzload(ssid, sizeof(ssid), offset));
  }

  //safe set of ssid, clips and ensures a trailing nul
  void setID(const char *s) {
    strzcpy(ssid, sizeof(ssid), s);
  }
  //safe set of password, clips and ensures a trailing nul
  void setPWD(const char *s) {
    strzcpy(password, sizeof(password), s);
  }

};

const char *Hostname = "980FMesher";

using MAC = uint8_t[WL_MAC_ADDR_LENGTH];

//will make into its own module real soon now.
struct Telnetter {
  bool amTrying = false;
  bool amConnected = false;
  bool beTrying = false;
  /** whether to send data from one wifi to all others as well as serial */
  bool chat = true;
  uint16_t teleport;//retained for diagnostics
  MonoStable testRate;

  const Credentials *cred = nullptr;

  wl_status_t wst = WL_NO_SHIELD;

  WiFiServer server;
  WiFiClient serverClients[MAX_SRV_CLIENTS];

  Telnetter(uint16_t teleport = 23): //23 is standard port, need to make this a configurable param so that we can serve different processes on one device.
    teleport(teleport),
    testRate(500), //500=2 Hz
    server(teleport) {
    //#done
  }

  /** you may call this in setup, or at some time later if you have some other means of getting credentials */
  bool begin(const Credentials &acred) {
    cred = &acred;
    beTrying = cred != nullptr;
    return tryConnect();//early first attempt, not strictly needed.
  }

  /** call this frequently from loop()
      Feeds merged wifi data from all clients to the given stream.
  */
  bool serve(Stream &stream) {//todo: use functional that matches Stream::write(uint8*,unsigned)
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
          stream.write(sbuf, len);
          if (chat) {
            broadcast(*sbuf, len, ci);
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
      WiFi.hostname(Hostname);
      WiFi.begin(cred->ssid, cred->password);
     
      dbg("\nDevice Mac ");
      MAC mac;
      WiFi.macAddress(mac);
      for (unsigned mi = 0; mi < WL_MAC_ADDR_LENGTH; mi++) {
        Char c(mac[mi]);
        dbg(':',c.hexNibble(1),c.hexNibble(0));
      }
      dbg("\nConnecting to AP ",cred->ssid);
      if (Verbose) {
        dbg(" using password ",cred->password);
      }
      amTrying = true;
    } else {
      amTrying = false;
    }
    return amTrying;
  }

  bool testConnection() {
    wst = WiFi.status();
    dbg("\tAPCon status ",wst);
    if (wst == WL_CONNECTED) {
      server.begin();
      server.setNoDelay(true);

      dbg("\nUse 'telnet ",WiFi.localIP());
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
    for (unsigned i = MAX_SRV_CLIENTS; i-- > 0;) {
      auto &aclient = serverClients[i];
      if (aclient && !aclient.connected()) {//connection died
        aclient.stop();//clean it up.
      }
      //not an else! aclient.stop() may make aclient be false.
      if (!aclient) {//if available
        aclient = server.available();
        dbg("\nNew client: #",i);//todo: find out how to get client ip, will need to go around wifi class proper to do so.

        aclient.write(Hostname);
        aclient.write(" at your service, station ");
        aclient.write('0'+i);
        aclient.write(".\n");
        return true;
      }
    }
    return false;
  }

} ;
