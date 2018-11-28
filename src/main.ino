#include <Arduino.h>

#ifdef ESP32
#include <ESP.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <HTTPClient.h>
#endif

#ifdef ESP8266
void syslog_it(char*);
#define DEBUG_HTTPCLIENT syslog_it

#include <ESP8266WiFi.h>
#include <ESP8266MDNS.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#endif

// config.h contains private information and is not distributed with
// the project files. Look for config-example.h and edit it to set
// things like Wifi SSID, IFTTT API Keys, and MQTT and REST API
// information
#include "config.h"

#include <ArduinoOTA.h>

#include "ina219_sensor.h"

INA219_Sensor ina219(UPDATE_DELAY, 0, 0, false);

#include "uptime.h"

Uptime uptime;

#include "heartbeat.h"

#include <Syslog.h>

WiFiUDP udpClient;
Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, APP_NAME, DEVICE_HOSTNAME);


#ifdef IFTTT_API_KEY
#include <IFTTTWebhook.h>
IFTTTWebhook ifttt(IFTTT_API_KEY, IFTTT_EVENT_NAME);
#endif

uint8_t RED = D6;
uint8_t GREEN = D5;
uint8_t BLUE = D0;

#define MAX_TARGET_STRING 1024
#define TARGET_RX_PIN D4
SoftwareSerial target(TARGET_RX_PIN, -1, false, MAX_TARGET_STRING);

#ifdef AIO_SERVER

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish power_feed(&mqtt, "/power");

Adafruit_MQTT_Publish uptime_feed(&mqtt, AIO_USERNAME "/feeds/furball.uptime");
Adafruit_MQTT_Publish freeheap_feed(&mqtt, AIO_USERNAME "/feeds/furball.freeheap");


void mqtt_connect(void) {
  int8_t ret;

  Serial.print("Connecting to MQTT server... ");

  while ((ret = mqtt.connect()) != 0) {
    switch (ret) {
      case 1: Serial.println("Wrong protocol"); break;
      case 2: Serial.println("ID rejected"); break;
      case 3: Serial.println("Server unavail"); break;
      case 4: Serial.println("Bad user/pass"); break;
      case 5: Serial.println("Not authed"); break;
      case 6: Serial.println("Failed to subscribe"); break;
      default: Serial.println("Connection failed"); break;
    }

    if(ret >= 0)
      mqtt.disconnect();

    Serial.println("Retrying connection...");
    delay(5000);
  }

  Serial.println("MQTT Connected!");
}
#endif


#ifdef IFTTT_API_KEY
#ifdef ESP32
#include <rom/rtc.h>

const char* reboot_reason(int code) {
  switch(code) {
    case 1 : return "POWERON_RESET";          /**<1, Vbat power on reset*/
    case 3 : return "SW_RESET";               /**<3, Software reset digital core*/
    case 4 : return "OWDT_RESET";             /**<4, Legacy watch dog reset digital core*/
    case 5 : return "DEEPSLEEP_RESET";        /**<5, Deep Sleep reset digital core*/
    case 6 : return "SDIO_RESET";             /**<6, Reset by SLC module, reset digital core*/
    case 7 : return "TG0WDT_SYS_RESET";       /**<7, Timer Group0 Watch dog reset digital core*/
    case 8 : return "TG1WDT_SYS_RESET";       /**<8, Timer Group1 Watch dog reset digital core*/
    case 9 : return "RTCWDT_SYS_RESET";       /**<9, RTC Watch dog Reset digital core*/
    case 10 : return "INTRUSION_RESET";       /**<10, Instrusion tested to reset CPU*/
    case 11 : return "TGWDT_CPU_RESET";       /**<11, Time Group reset CPU*/
    case 12 : return "SW_CPU_RESET";          /**<12, Software reset CPU*/
    case 13 : return "RTCWDT_CPU_RESET";      /**<13, RTC Watch dog Reset CPU*/
    case 14 : return "EXT_CPU_RESET";         /**<14, for APP CPU, reseted by PRO CPU*/
    case 15 : return "RTCWDT_BROWN_OUT_RESET";/**<15, Reset when the vdd voltage is not stable*/
    case 16 : return "RTCWDT_RTC_RESET";      /**<16, RTC Watch dog reset digital core and rtc module*/
    default : return "NO_MEAN";
  }
}
#endif
#endif

char powerball_hostname[sizeof(FURBALL_HOSTNAME) + 8];

void setup() {
  byte mac_address[6];

  delay(2000);

  Serial.begin(115200);
  Serial.println("Hello World!");

  target.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  WiFi.macAddress(mac_address);
  snprintf(powerball_hostname, sizeof(powerball_hostname), "%s-%02x%02x%02x", FURBALL_HOSTNAME, (int)mac_address[3], (int)mac_address[4], (int)mac_address[5]);
  Serial.printf("Hostname is %s\n", powerball_hostname);

#ifdef ESP32
  WiFi.setHostname(powerball_hostname);
#endif
  while(!WiFi.isConnected()) {
    Serial.print(".");
    delay(100);
  }

  Serial.println();
  Serial.println("Connected!");

#ifdef IFTTT_API_KEY
  if(ifttt.trigger("reboot", powerball_hostname, WiFi.localIP().toString().c_str())) {
    Serial.println("IFTTT failed");
    syslog.log(LOG_INFO, "IFTTT failed");
  } else {
    Serial.println("IFTTT notified");
    syslog.log(LOG_INFO, "IFTTT notified");
  }
#endif

  syslog.logf(LOG_INFO, "%s booted!", powerball_hostname);

  if(!MDNS.begin(powerball_hostname))
    Serial.println("Error setting up MDNS responder!");
  else
    Serial.println("mDNS responder started");

  Serial.println("MQTT");
  mqtt_connect();

#ifdef ESP32
   ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
      ESP.restart();
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
#endif

  ArduinoOTA.begin();

  ina219.begin();

  Serial.println();

  delay(1000);
}

struct power_info {
  float voltage;
  float current;
  uint32_t millis;
};

#define MAX_INFOS 10

struct power_info infos[MAX_INFOS];
uint16_t current_infos = 0;

void loop() {
  static unsigned long last_loop = 0;
  static char serial_buffer[MAX_TARGET_STRING+1];
  static uint8_t serial_buffer_index = 0;

  while(target.available() > 0) {
    char c = target.read();
    if(c == '\n') {
      Serial.println(serial_buffer);
      syslog.log(LOG_INFO, serial_buffer);
      serial_buffer_index = 0;
    } else {
      serial_buffer[serial_buffer_index++] = c;
      serial_buffer[serial_buffer_index] = '\0';
    }
  }

  ina219.handle();

  if(ina219.ready_for_update()) {
    // #define VERBOSE 1
#ifdef VERBOSE
    Serial.printf("%6.2fv %6.2fma %7.2fmw\n", ina219.load_voltage(), -ina219.current(), ina219.power());
#endif

#ifdef AIO_SERVER
    if(! mqtt.ping(3)) {
      if(! mqtt.connected())
	mqtt_connect();
    }

    infos[current_infos].voltage = ina219.load_voltage();
    infos[current_infos].current = -ina219.current();
    infos[current_infos].millis = millis();

    //    Serial.printf("%d\n", current_infos);

    if(++current_infos == MAX_INFOS) {
      char buf[32*MAX_INFOS];

      buf[0] = '\0';

      for(int i = 0; i < MAX_INFOS; i++)
	snprintf(buf + strlen(buf), 23, "%.2f,%.2f,%u\n", infos[i].voltage, infos[i].current, infos[i].millis);

      current_infos = 0;

#if 0
      Serial.print("MQTT: ");
      Serial.println(buf);

      Serial.println("syslog");
      syslog.log(LOG_INFO, buf);

      Serial.println("mqtt");
#endif

      power_feed.publish(buf);

      //      Serial.println("mqtt done");
    }

#endif
  }

  ArduinoOTA.handle();

  if(millis() - last_loop < UPDATE_DELAY)
    return;

  last_loop = millis();

#ifdef AIO_SERVERX
  if(! mqtt.ping(3)) {
    if(! mqtt.connected())
      mqtt_connect();
  }

  uptime_feed.publish((unsigned)uptime.uptime()/1000);
  freeheap_feed.publish(ESP.getFreeHeap());
#endif

#ifdef VERBOSEX
  Serial.printf("Uptime %.2f seconds\n", uptime.uptime() / 1000.0);
  Serial.printf("Free heap %u bytes\n", ESP.getFreeHeap());
#endif

#ifdef REST_API_ENDPOINT
  char buffer[500];
  snprintf(buffer, 500, "{\"temperature\": %d, \"humidity\": %d, \"pressure\": %d, \"eco2\": %d, \"tvoc\": %d, \"pm1\": %d, \"pm25\": %d, \"pm10\": %d, \"freeheap\": %d, \"uptime\": %lu, \"lux\": %d, \"full_light\": %d, \"ir\": %d, \"visible\": %d }",
	   );

#ifdef VERBOSE
    Serial.println(buffer);
#endif

    post(buffer);
#endif
}

#ifdef REST_API_ENDPOINT
void post(char *json) {
  HTTPClient http;

  http.begin(String(REST_API_ENDPOINT));
  http.addHeader("Content-Type", "application/json");
  int response = http.POST(json);

#ifdef VERBOSE
  if(response > 0) {
    Serial.printf("HTTP status code %d\n", response);
  } else {
    Serial.printf("HTTPClient error %d\n", response);
  }
#endif

  http.end();
}
#endif
