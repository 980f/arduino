
#pragma once


//max number of simultaneous clients allowed, note: their incoming stuff is mixed together. using 3 as the ESP8266 supports at most 4 sockets.
#define MAX_SRV_CLIENTS 3

#include <WiFi.h>
#ifndef WL_SSID_MAX_LENGTH
#warning "WiFi.h did not define SSID and WPA max lengths"
#define WL_SSID_MAX_LENGTH 32
#define WL_WPA_KEY_MAX_LENGTH 63
#endif

struct Credentials {
  char ssid[WL_SSID_MAX_LENGTH];
  char password[WL_WPA_KEY_MAX_LENGTH];

  unsigned save(unsigned offset = 0); 

  unsigned load(unsigned offset = 0);

  /** safe set of ssid, clips and ensures a trailing nul */
  Credentials& setID(const char *s);

  /**safe set of password, clips and ensures a trailing nul */
  Credentials& setPWD(const char *s);

};


/** end user class for telnet service */
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


#include "millievent.h"
/** a server that rebroadcasts whatever it receives either from a wifi client or the local serial port */
struct Telnetter {
  bool amTrying = false;
  bool amConnected = false;
  bool beTrying = false;
  static bool Verbose;  //extra spew

  uint16_t teleport;//retained for diagnostics
  MonoStable testRate; //rate limiter for wifi access port connection checks

  const Credentials *cred = nullptr;

  wl_status_t wst = WL_NO_SHIELD;

  WiFiServer server;
  WiFiClient serverClients[MAX_SRV_CLIENTS];

  TelnetActor *actor;

  Telnetter(TelnetActor *actor, uint16_t teleport = 23);

  /** you may call this in setup, or at some time later if you have some other means of getting credentials */
  bool begin(const Credentials &acred);


  /** call this frequently from loop()
      calls back with chunks from clients.
      Feeds merged wifi data from all clients to the given stream.
  */
  bool serve( );//todo: use functional that matches Stream::write(uint8*,unsigned) instead of TelnetActor

  /** sends given data to all clients */
  void broadcast(const uint8_t &sbuf, const unsigned len, const unsigned sender = ~0U);

  /** sends data from given stream to all clients */
  void broadcast(Stream & stream);

//internals, left public for debug
  bool tryConnect();
  bool testConnection();

  /** see if we have a place to keep client connection info. If so retain it.*/
  bool findSlot();

} ;
