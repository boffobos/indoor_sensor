//Loading config from file system and save to

bool saveConfig() {
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config.json file");
    configFile.close();  
    return false;    
  }

  StaticJsonDocument<300> configJson;
  configJson["email"] = config.email;
  configJson["room"] = config.room;
  configJson["ssid"] = config.ssid;
  configJson["password"] = config.password;
  configJson["isWifiValid"] = config.isWifiValid;
  configJson["mac"] = config.mac;
  configJson["altServerAddress"] = config.altServerAddress;

  if (serializeJson(configJson, configFile) == 0) {
    Serial.println("Failed to write config file");
    configFile.close();
    return false;  
  }
  Serial.println("Config file has been written successfully!");
  configFile.close(); 
  configChandedTrigger();  
  return true;
}

//Loading config from file system and load it to RAM
bool loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to load config file");
    configFile.close();
    return false;    
  }
  
  StaticJsonDocument<300> configJson;
  String buffer = configFile.readString();     
  DeserializationError e = deserializeJson(configJson,buffer);
  
  if(!e) {
    config.email = String(configJson["email"]);
    config.room = String(configJson["room"]);
    config.ssid = String(configJson["ssid"]);
    config.password = String(configJson["password"]);
    config.isWifiValid = String(configJson["isWifiValid"]).equals("true");
    configFile.close();
    return true;
  } else {
    Serial.println("Failed to read config file");
    configFile.close();
    return false;
  }
  
}

// Function for adding nessesary actions after saving config
void configChandedTrigger() {
  loadConfig();
  
  if (WiFi.isConnected()) {
    
  } else {
    if(config.isWifiValid) {
      if (!initWiFi(config.ssid.c_str(), config.password.c_str())) {
        initWiFiAP();
      } else {
        if (WiFi.softAPdisconnect(true)) {
          Serial.println("Soft AP disabled!");
        }
      }
    } else {
      initWiFiAP();
    }

  }
    
}

