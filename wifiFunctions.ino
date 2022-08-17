// Initialize WiFi
bool initWiFi(const char *ssid, const char *password) {
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

    Serial.println("");   
    if(WiFi.status() == WL_WRONG_PASSWORD || WiFi.status() == WL_NO_SSID_AVAIL) {
      config.isWifiValid = false;
      saveConfig();
      Serial.println("Can not connect to WiFi.");  
      return false;  
    }
    IPAddress ip = WiFi.localIP();    
    Serial.println(ip);
    if (!config.isWifiValid) {
      config.isWifiValid = true;
      saveConfig();
    }
    return true;
  }
  return false;
}

// Configuring wifi sort AP in case first start and requesting credentials
void initWiFiAP() {
  config.mac = WiFi.macAddress();
  char* ssidPrefix = "Sensor_";
  String ssidPostfix = WiFi.macAddress().substring(8); 
  ssidPostfix.replace(":", "");  
  strcat(ssidPrefix, ssidPostfix.c_str());
  const char *APssid = ssidPrefix;
  const char *APpassword = "12345678";
  IPAddress local_IP(192,168,90,1);
  
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, local_IP, IPAddress(255,255,255,0)) ? "Ready" : "Failed!");
  WiFi.softAP(APssid, APpassword ? APpassword : NULL);
  Serial.print("AP soft IP address: ");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println(myIP);

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", local_IP);
  
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); //only when requested from AP
  
}