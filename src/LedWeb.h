#ifndef __LED_WEB_H__
#define __LED_WEB_H__

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#include "MicroUtil.h"
#include "html/html.h"
#include "LedControl.h"

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

#define JSON_BUFFER_CAPACITY_BYTES 200

class LedWeb {
public:
  LedWeb() {};

  void begin(LedControl *led_ctrl) {
    this->led_ctrl = led_ctrl;
    wifi_setup();
  }

  void handle() {
    reset_wifi_if_not_connected();
    if (!connected) return;

    server->handleClient();
  }

  void handle_request() {
    #ifdef ENABLE_SERIAL_DEBUG
      Serial.print("request uri: ");
      Serial.println(server->uri());
    #endif

    // Effectively disable CORS on every request.
    server->sendHeader("Access-Control-Allow-Origin", "*");

    switch(shash(server->uri().c_str())) {
      case shash("/"):
        serve_static(PAGE_MAIN);
        break;
      case shash("/api"):
        // "plain" is a special argument in post request to access the raw body
        if (server->hasArg("plain")) {
          const String body = server->arg("plain");
          DeserializationError err = deserializeJson(doc, body);
          if (err) {
            #ifdef ENABLE_SERIAL_DEBUG
              Serial.print(F("JSON deserialization failed: "));
              Serial.println(err.c_str());
            #endif

            serve_server_error();
          }
          handle_api_request();
        } else {
          serve_bad_request();
        }
        break;
      default:
        serve_static("Not Found", 404);
    }
  }

private:
  LedControl* led_ctrl = nullptr;
  long last_connection_attempt = -1;
  bool connected = false;

  ESP8266WebServer *server = nullptr;
  DynamicJsonDocument doc = DynamicJsonDocument(JSON_BUFFER_CAPACITY_BYTES);

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
          server->on("/", HTTP_GET, std::bind(&LedWeb::handle_request, this));
          server->on("/api", HTTP_POST, std::bind(&LedWeb::handle_request, this));
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

  void serve_static(const char *content,
                    int http_code = 200,
                    const char *mime_type = "text/html") {
    server->send(http_code, mime_type, content);
  }

  void serve_server_error() {
    serve_static("Server Error", 500);
  }

  void serve_bad_request() {
    serve_static("Bad Request", 400);
  }

  void handle_api_request() {
    const char* op = doc["op"]; // e.g. "fill_solid"

    switch(shash(op)) {
      case shash("fill_solid"):
        handle_fill_solid();
        break;
      case shash("status"):
        handle_status();
        break;
      case shash("reboot"):
        api_response_success();
        // wait 1 seconds before actually killing the system so that we
        // can serve the response
        delay(1000);
        ESP.restart();
        break;
      case shash("set_brightness"):
        handle_brightness();
        break;
      default:
        #ifdef ENABLE_SERIAL_DEBUG
          Serial.print(F("Invalid op requested: "));
          Serial.println(op);
        #endif
        serve_bad_request();
    }
  }

  inline void api_response_success() {
    serve_static("{ \"success\": true }", 200, "application/json");
  }

  void handle_fill_solid() {
    const int range_start = doc["range_start"] | 0;
    const int range_size = doc["range_size"] | led_ctrl->num_leds;

    if (!doc["color"].is<JsonArray>() || doc["color"].as<JsonArray>().size() != 3) {
      serve_bad_request();
      return;
    }

    JsonArray color = doc["color"];
    const uint8_t color_r = color[0];
    const uint8_t color_g = color[1];
    const uint8_t color_b = color[2];

    const CRGB crgb = CRGB(color_r, color_g, color_b);
    led_ctrl->fill_solid(crgb, range_start, range_size);
    led_ctrl->commit();

    api_response_success();
  }

  void handle_brightness() {
    if (!doc["value"].is<int>()) {
    serve_bad_request();
      return;
    }
    uint8_t value = doc["value"] | 0;
    led_ctrl->set_brightness(value);
    api_response_success();
  }

  void handle_status() {
    StaticJsonDocument<JSON_OBJECT_SIZE(2) + JSON_ARRAY_SIZE(60) + 60 * 8> response;
    response["brightness"] = led_ctrl->get_brightness();
    JsonArray leds = response.createNestedArray("leds");

    for (int16_t i = 0; i < led_ctrl->num_leds; i++) {
      const int32_t crgb =
        (this->led_ctrl->leds[i].r << 16) +
        (this->led_ctrl->leds[i].g << 8) +
        this->led_ctrl->leds[i].b;
      leds.add(String("#") + String(crgb, HEX));
    }

    String output;
    serializeJson(response, output);

    serve_static(output.c_str(), 200, "application/json");
  }
};

#endif // __LED_WEB_H__
