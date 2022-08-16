  // Send HTTP response with sensor data.
void sendSensorData() {
  
  WiFiClient client;
  HTTPClient http;
  String body = getSensorReadings();


  Serial.print("[HTTP] begin...\n");
  // configure target server and url
  http.begin(client, serverAddress, serverPort, serverPath);  //conside to refactor to set server address
  http.addHeader("Content-Type", "application/json");

  Serial.print("[HTTP] POST...\n");
  // start connection and send HTTP header and body

  int httpCode = http.POST(body);

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] POST... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      // Here it is possible to handle server response with config data;
      const String &payload = http.getString();
      Serial.println("received payload:\n<<");
      Serial.println(payload);
      Serial.println(">>");
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}