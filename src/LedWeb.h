#ifndef __LED_WEB_H__
#define __LED_WEB_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "MicroUtil.h"
#include "html/html.h"

#define WIFI_HOSTNAME QUOTE(_WIFI_HOSTNAME)
#ifndef _WIFI_SSID
#  pragma message "No WIFI SSID defined: using default value of 'ledbox'"
#  define WIFI_SSID "ledbox"
#else
#  define WIFI_SSID QUOTE(_WIFI_SSID)
#endif
#ifndef _WIFI_PASS
#  pragma message "No WIFI pass key defined: using default value of 'password'"
#  define WIFI_PASS "password"
#else
#  define WIFI_PASS QUOTE(_WIFI_PASS)
#endif

class LedWeb {
public:
  LedWeb() {};

  void begin() {
    wifi_setup();
  }

  void handle() {
    reset_wifi_if_not_connected();
    if (!connected) return;

    server->handleClient();
  }

  void handle_main() {
    server->send(200, "text/html", PAGE_MAIN);
  }
private:
  long last_connection_attempt = -1;
  bool connected = false;

  ESP8266WebServer *server = nullptr;


  void wifi_setup(uint32_t delay_ms = 30000) {
    const uint32_t now = millis();

    if (last_connection_attempt < 0 || now - last_connection_attempt >= delay_ms) {
      #ifdef ENABLE_SERIAL_DEBUG
        Serial.print("Attempt connection to WiFi: '");
        Serial.print(WIFI_SSID);
        Serial.println("'...");
      #endif
      WiFi.hostname(WIFI_HOSTNAME);
      WiFi.begin(WIFI_SSID, WIFI_PASS);

      last_connection_attempt = now;
    }
  }

  void reset_wifi_if_not_connected() {
    const wl_status_t wifi_status = WiFi.status();

    switch (wifi_status) {
      case WL_CONNECT_FAILED:
      case WL_CONNECTION_LOST:
      case WL_NO_SSID_AVAIL:
        if (connected) {
          #ifdef ENABLE_SERIAL_DEBUG
            Serial.print("Lost connection to WIFI:");
            Serial.println(wifi_status);
          #endif
          connected = false;
        }
        wifi_setup();
        break;
      case WL_CONNECTED:
        if (!connected) {
          #ifdef ENABLE_SERIAL_DEBUG
            Serial.println("Connected to WiFi.");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
          #endif

          if (server != NULL) {
            server->close();
            delete server;
          }

          server = new ESP8266WebServer(80);
          server->on("/main.html", HTTP_GET, std::bind(&LedWeb::handle_main, this));
          server->begin();
        }
        connected = true;
        return;
      default:
        // We only care about the states above but add a 'default' block here
        // so that we lose compiler warnings.
        break;
    }
  }
};

#endif // __LED_WEB_H__
