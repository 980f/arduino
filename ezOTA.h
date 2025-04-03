#pragma once
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

#include <WiFiUdp.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#include <NetworkUdp.h>
#else
#error this code only works for ESP 8266 or 32
#endif
#include <esp_mac.h>
#include <ArduinoOTA.h>

/** todo: non-blocking, timeout in onTick, optionally loop over list forever
  also create an OTA server esp32 or raspberry pi.
*/

struct Credential {
  const char *ssid;
  const char *psk;
  Credential(const char *ssid, const char *psk): ssid(ssid), psk(psk) {}
};

std::array EzOTA_Creds = {
  Credential {"alice-fxtracker", "SpecialEffects"},
  Credential {"omada", "scareforacure"},
  Credential {"Grandpadee", "6440orfight"},  
  Credential {"honeyspot", "brigadoonwillbebacksoon"},
};


struct EzOTA {

  bool startWifi(byte *ownAddress) {
    if (WiFi.isConnected()) {
      Serial.printf("Wifi Start attempted when already connected\n");
      return true;
    }
    for (Credential &creds : EzOTA_Creds) {
      WiFi.mode(WIFI_STA);
      Serial.printf("Trying %s:%s\n", creds.ssid, creds.psk);
      WiFi.begin(creds.ssid, creds.psk);
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
        if (ownAddress) {
          WiFi.macAddress(ownAddress);
          Serial.print("I am: ");
          for (unsigned i = 0; i < 6; ++i) {
            Serial.printf(":%02x", ownAddress[i]);
          }
          Serial.println();
        }

        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return true;
      } else {
        if (WiFi.isConnected()) {
          Serial.printf("Wifi is connected to %s despite wfcr saying otherwise\n", creds.ssid);
          return true;
        }
      }
    }//try another

    return false;
  }

  void enableOTA() {
    // Port defaults to 8266 or 3232
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else {  // U_FS
        type = "filesystem";
      }

      // NOTE: if updating FS this would be the place to unmount FS using FS.end()
      Serial.println("\nStart updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        Serial.println("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        Serial.println("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        Serial.println("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        Serial.println("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        Serial.println("End Failed");
      }
    });
    ArduinoOTA.begin();
  }


  void setup(byte *macBuffer = nullptr) {
    startWifi(macBuffer);
    enableOTA();
    Serial.println("ezOTA Setup Complete\n\n");
  }


  void loop() {
    ArduinoOTA.handle();
  }

  void onTick() {
    //will eventually move setup into background and perhaps time it out here.
  }
};
