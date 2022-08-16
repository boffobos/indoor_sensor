#include <Arduino.h>
#include "LittleFS.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h> 
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_AHT10.h>
#include <Adafruit_Sensor.h>

struct {
  String email;
  String room;
  String ssid;  
  String password;  
  bool isWifiValid;
  String altServerAddress;
  String mac;
} config;


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// DNS server
DNSServer dnsServer;
const byte DNS_PORT = 53;

// Create an Event Source on /events
AsyncEventSource events("/events");

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
unsigned long softAPTimeout = 120000; //if no one connects Soft_AP wifi mode changes to station;
unsigned long postSensorDataInterval = 120000;
unsigned long lastTimePost = 0;

const char *serverAddress = "192.168.1.18";
const int serverPort = 3000;
const char *serverPath = "/post-sensor-data";

// Create a sensor object
Adafruit_AHT10 aht;               // AHT10 connect to ESP32 I2C (GPIO 21 = SDA, GPIO 22 = SCL)
const int sensorIterations = 10;  //check this number of sensor data and compute average calculation

// Init BME280
void initBME() {
  if (!aht.begin()) {
    Serial.println("Could not find a valid AHT10 sensor, check wiring!");
    while (1) delay(10);
  }
}

// Handler for Soft AP mode to redirect to captive portal
class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {
    server.on(
      "/config", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        // first callback
      },
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        StaticJsonDocument<200> json;
        DeserializationError e = deserializeJson(json, (char *)data);
        if (!e) {
          Serial.println(String(json["email"]));
          File config = LittleFS.open("/config.json", "w");
          if (!config) {
            request->send(204, "Unable to create file, try again");
            return;              
          }  
          if (serializeJson(json, config) == 0) {
            request->send(204, "Failed to write config file. Try again!");
            Serial.println(config.readString());
            config.close();
            return;
          } 
          request->send(200);
        } else {
          request->send(204);
        }            
      }
    );
  
    server.serveStatic("/", LittleFS, "/");
    
  }
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }  
  
  void handleRequest(AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  }
};



// Get Sensor Readings and return JSON object
String getSensorReadings() {
  sensors_event_t humidity, temp;

  aht.getEvent(&humidity, &temp);
  int i;
  float ta = 0;
  float ha = 0;

  for (i = 0; i < sensorIterations; i++) {
    ta += temp.temperature;
    ha += humidity.relative_humidity;
    delay(10);
  }

  ta = ta / sensorIterations;
  ha = ha / sensorIterations;

  StaticJsonDocument<250> doc;
  doc["temperature"] = ta;
  doc["humidity"] = ha;
  doc["mac"] = config.mac;
  doc["email"] = config.email;
  doc["room"] = config.room;

  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Initialize WiFi
void initWiFi(const char *ssid, const char *password) {
  config.mac = WiFi.macAddress();
  if (ssid && password) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi ..");
    int timeout = 60;  
    while (WiFi.status() != WL_CONNECTED && timeout) {
      timeout--;
      Serial.print('.');
      delay(1000);
    }
    if(WiFi.status() == WL_WRONG_PASSWORD || WiFi.status() == WL_NO_SSID_AVAIL) {
      config.isWifiValid = false;
      initWiFiAP();
    }
    Serial.println(WiFi.localIP());
        
  }
  // if (MDNS.begin("tempSensor")) {
  //   Serial.println("MDNS responder started");
  // }
}

void initWiFiAP() {
  config.mac = WiFi.macAddress();
  /* Working out AP SSID as Sensor_last_3_mac_octets */  
  // Serial.println(mac);
  // char* buffer = "Sensor_";
  // strcat(buffer, mac.c_str());
  // Serial.println(buffer);
  // const char *APssid = buffer;
  const char *APssid = "Sensor_B6:47:0F_AP";
  const char *APpassword = "12345678";
  IPAddress local_IP(192,168,90,1);
  
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, local_IP, IPAddress(255,255,255,0)) ? "Ready" : "Failed!");
  WiFi.softAP(APssid, APpassword ? APpassword : NULL);
  // delay(500); //for seeing localIP
  Serial.print("AP soft IP address: ");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", local_IP);
  
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); //only when requested from AP
  
}

void setup() {
  Serial.begin(115200);
  initBME();
  initFS();
  if(!loadConfig()) {
    initWiFiAP();
  } else {
    initWiFi(config.ssid.c_str(), config.password.c_str());   
  }
  
  
  // initWiFi();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // API for the latest sensor readings
  server.on("/get-data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = getSensorReadings();
    request->send(200, "application/json", json);
    json = String();
  });
  
  server.on("/get-config", HTTP_GET, [](AsyncWebServerRequest *request) {
    File config = LittleFS.open("/config.json", "r");
    if (!config) {
      request->send(204, "File to open config file.");
      return;
    }
    String json = config.readString();
    request->send(200, "application/json", json);
  });

  server.on(
    "/config", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      // first callback
    },
    NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      StaticJsonDocument<200> configJson;
      DeserializationError e = deserializeJson(configJson, (char *)data);
      if (!e) {
         config.room = String(configJson["room"]);       
         config.email = String(configJson["email"]);       
         config.ssid = String(configJson["ssid"]);       
         config.password = String(configJson["password"]);       
         config.altServerAddress = String(configJson["altServerAddress"]);       
          
        if (!saveConfig()) {
          request->send(204, "Failed to save data, try again");
          return;              
        }  
        request->send(200);
                
      } else {
        request->send(204, "Failed to recognize data. Check JSON format available only");
      }            
  });
  // for development purpose only
  server.on("/reset-config", HTTP_GET, [](AsyncWebServerRequest *request) {
      if(LittleFS.remove("/config.json")) {
        request->send(200, "Config removed");    
        return;
      }
    request->send(204, "reset config failed, try again");    
  });  

  events.onConnect([](AsyncEventSourceClient *client) {
    if (client->lastId()) {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);

  // Start server
  server.begin();

}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Send Events to the client with the Sensor Readings Every 30 seconds
    String body = getSensorReadings();
    events.send("ping", NULL, millis());
    events.send(body.c_str(), "new_readings", millis());
    lastTime = millis();
  }

  if ((millis() - lastTimePost) > postSensorDataInterval) {
    if ((WiFi.status() == WL_CONNECTED)) {
      sendSensorData();
      lastTimePost = millis();
    }
  }
  dnsServer.processNextRequest();
  
}
