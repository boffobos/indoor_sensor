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
  return true;
}

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
    // const char *ssid = configJson["ssid"];
    // const char *password = configJson["password"];
    config.email = String(configJson["email"]);
    config.room = String(configJson["room"]);
    config.ssid = String(configJson["ssid"]);
    config.password = String(configJson["password"]);
    config.isWifiValid = String(configJson["isWifiValid"]).compareTo("true") == 0 ? true : false;
    configFile.close();
    return true;
  } else {
    Serial.println("Failed to read config file");
    configFile.close();
    return false;
  }
  
}

