void loadConfiguration() {
  // Open the configuration file for reading
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println(F("Failed to open config file for reading"));
    return;
  }

  // Parse the JSON document from the configuration file
  const size_t capacity = JSON_ARRAY_SIZE(SOLENOID_COUNT) + SOLENOID_COUNT*JSON_OBJECT_SIZE(2);
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println(F("Failed to parse config file"));
    return;
  }

  // Close the configuration file
  configFile.close();

  // Get the userMax and userMin arrays from the JSON document
  JsonArray maxArray = doc["userMax"];
  if (maxArray.size() != MUX_COUNT) {
    Serial.println(F("Invalid userMax array size"));
    return;
  }
  for (int i = 0; i < MUX_COUNT; i++) {
    userMax[i] = maxArray[i];
  }
  JsonArray minArray = doc["userMin"];
  if (minArray.size() != MUX_COUNT) {
    Serial.println(F("Invalid userMin array size"));
    return;
  }
  for (int i = 0; i < MUX_COUNT; i++) {
    userMin[i] = minArray[i];
  }

  // Print the loaded user-defined maximum and minimum values for each sensor to the serial monitor
  Serial.println("Loaded user-defined soil humidity thresholds:");
  for (int i = 0; i < MUX_COUNT; i++) {
    Serial.print("Sensor "); Serial.print(i); Serial.print(": Max = "); Serial.print(userMax[i]); Serial.print(", Min = "); Serial.println(userMin[i]);
  }
}

void saveConfiguration() {
  // Créer un nouveau document JSON
  DynamicJsonDocument doc(1024);
  
    // Add the userMax and userMin arrays to the JSON document
  JsonArray maxArray = doc.createNestedArray("userMax");
  for (int i = 0; i < SOLENOID_COUNT; i++) {
    maxArray.add(userMax[i]);
  }
  JsonArray minArray = doc.createNestedArray("userMin");
  for (int i = 0; i < SOLENOID_COUNT; i++) {
    minArray.add(userMin[i]);
  }

  // Open the configuration file for writing
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println(F("Failed to open config file for writing"));
    return;
  }

  // Serialize the JSON document to the configuration file
  if (serializeJson(doc, configFile) == 0) {
    Serial.println(F("Failed to write to config file"));
  }

  // Close the configuration file
  configFile.close();
  Serial.println("Data saved to file");
}
