void readSensors() {
  for (int mux = 0; mux < MUX_COUNT; mux++) {
    int sensorSum = 0; // variable to store the sum of sensor values
    mcpMuxChip.digitalWrite(MUX_PINS[mux], LOW);
    for (int channel = 0; channel < numSensorsMux[mux]; channel++) {
      digitalWrite(S0, channel & 0x01);
      digitalWrite(S1, channel & 0x02);
      digitalWrite(S2, channel & 0x04);
      digitalWrite(S3, channel & 0x08);
      digitalWrite(MUX_A, channel & 0x10);
      digitalWrite(MUX_B, channel & 0x20);
      sensorValuesMux[mux][channel] = analogRead(ANALOG_PIN);
      // show the sensor value
      //Serial.print("Mux Number: "); Serial.print(mux); Serial.print("\tChannel Number: "); Serial.print(channel);
      //Serial.print("\tSensor Value: "); Serial.println(sensorValuesMux[mux][channel]);
      sensorSum += sensorValuesMux[mux][channel];
    }
    int sensorAverage = (numSensorsMux[mux] > 0) ? (sensorSum / numSensorsMux[mux]) : 0; // calculate the average sensor value for this mux
    sensorValuesTotal[mux] = sensorAverage; // store the total average sensor value for this mux
    mcpMuxChip.digitalWrite(MUX_PINS[mux], HIGH);
  }
}

void controlSolenoids() {
  boolean anySolenoidOpen = false; // Variable to keep track if any solenoid is open

  for (int mux = 0; mux < MUX_COUNT; mux++) { // Loop through all multiplexers
    for (int channel = 0; channel < MAX_SENSOR_PER_MUX; channel++) {
      int i = mux * MAX_SENSOR_PER_MUX + channel;
      if (mux < MUX_COUNT && channel < numSensorsMux[mux]) {
        if (sensorValuesMux[mux][channel] > userMax[i]) {
          digitalWrite(SOLENOID_PINS[i], LOW); // Close solenoid
          solenoidStatus[i] = false; // Update solenoid status
        } else if (sensorValuesMux[mux][channel] < userMin[i]) {
          digitalWrite(SOLENOID_PINS[i], HIGH); // Open solenoid
          solenoidStatus[i] = true; // Update solenoid status
          anySolenoidOpen = true; // Set variable to true
        }
      }
    }
  }

  if (anySolenoidOpen) {
    digitalWrite(PUMP_PIN, HIGH); // Turn on pump
    pumpStatus = true; // Update pump status
  } else {
    digitalWrite(PUMP_PIN, LOW); // Turn off pump
    pumpStatus = false; // Update pump status
  }
}
