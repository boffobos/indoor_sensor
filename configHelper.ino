//Loading config from file system and save to

bool saveConfig() {
  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config.json file");
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
    return false;  
  }
  Serial.println("Config file has been written successfuly!");  
  return true;
}

bool loadConfig() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to load config file");
    return false ;    
  }
  
  StaticJsonDocument<300> configJson;
  String buffer = configFile.readString();     
  DeserializationError e = deserializeJson(configJson,buffer);
  
  if(!e) {
    // const char *ssid = configJson["ssid"];
    // const char *password = configJson["password"];
    config.email = String(configJson["email"]);
    config.room = String(configJson["room"]);
    config.ssid = String(configJson["ssid"]);
    config.password = String(configJson["password"]);
    config.isWifiValid = String(configJson["password"]) == "true" ? true : false;
    config.altServerAddress = String(configJson["altServerAddress"]);
    
    configFile.close();
    return true;
    // if (ssid && password) {
    //   initWiFi(ssid, password);
    //   return true;
    // }
    // return false;
  } else {
    Serial.println("Failed to read config file");
    configFile.close();
    return false;
  }
  
}

