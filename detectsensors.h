void detectSensors() {
  for (int mux = 0; mux < MUX_COUNT; mux++) {
    mcpMuxChip.digitalWrite(MUX_PINS[mux], LOW);
    int numSensors = 0;
    for (int channel = 0; channel < MAX_SENSOR_PER_MUX; channel++) {
      digitalWrite(S0, channel & 0x01);
      digitalWrite(S1, channel & 0x02);
      digitalWrite(S2, channel & 0x04);
      digitalWrite(S3, channel & 0x08);
      digitalWrite(MUX_A, channel & 0x10);
      digitalWrite(MUX_B, channel & 0x20);
      int sensorValue = analogRead(ANALOG_PIN);
      if (sensorValue > 0) {
        numSensors++;
      }
    }
    numSensorsMux[mux] = numSensors;
    // view how many sensor detected by MUX
    //Serial.print("sensor detected MUX: "); Serial.print(mux);
    //Serial.print("\tnumber of sensor: "); Serial.println(numSensorsMux[mux]);
  
    mcpMuxChip.digitalWrite(MUX_PINS[mux], HIGH);
    // Allocate memory for the sensor values for each mux
      for (int i = 0; i < numSensorsMux[mux]; i++) {
        sensorValuesMux[mux][i] = 0;
      }
  }
}
