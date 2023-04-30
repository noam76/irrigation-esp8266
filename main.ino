#include <Arduino.h>
#include <Wire.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Adafruit_MCP23X17.h>

// Définir le temps d'intervalle de la fonction de ticker
const unsigned long tickerInterval = 25;

#define S0 D0 // Assign Multiplexer pin S0 connect to pin D0 of NodeMCU
#define S1 D7 // Assign Multiplexer pin S1 connect to pin D3 of NodeMCU
#define S2 D8 // Assign Multiplexer pin S2 connect to pin D7 of NodeMCU
#define S3 D3 // Assign Multiplexer pin S3 connect to pin D8 of NodeMCU
#define ANALOG_PIN A0 // Assign ANALOG_PIN read from analog multiplexer

#define PUMP_PIN D6 // define pin D6 to control pump

// Define mux channels pin control for all 16 mux cd74hc4051 to read only the first 4 channels of each MUX
#define MUX_A D4 
#define MUX_B D5
// #define MUX_C 4

Adafruit_MCP23X17 mcpSolenoid, mcpMuxChip;

const int MAX_SENSOR_PER_MUX = 4; // maximum number of sensors per MUX
const int SOLENOID_COUNT = 15; // number of solenoids connected to MCP23017 #1
const int MUX_COUNT = 15; // number of MUX cd74hc4051 connected to MCP23017 #2

// Définition des broches de sortie pour chaque solénoïde
const int SOLENOID_PINS[SOLENOID_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}; // MCP23017 pins connected to solenoids
const int MUX_PINS[MUX_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}; // MCP23017 pins connected to cd74hc4051
int numSensorsMux[MUX_COUNT]; // Tableau contenant le nombre de capteurs connectés à chaque multiplexeur
int sensorValuesMux[MUX_COUNT][MAX_SENSOR_PER_MUX]; // 2D array to store sensor values from all multiplexers
int sensorValuesTotal[MUX_COUNT]; // array to store sensor value total of each MUX
int userMax[MUX_COUNT]; // Define array to store user-defined maximum values
int userMin[MUX_COUNT]; // Define array to store user-defined minimum values
bool anySolenoidsActive = false; // flag to indicate if any solenoids are active
bool solenoidStatus[SOLENOID_COUNT];
bool pumpStatus;

ESP8266WebServer server(80);
#include "html.h"
#include "detectsensor.h"
#include "readsensors.h"
#include "jsonconfig.h"

void handleRoot(){
  server.send_P(200, "text/html", INDEX_HTML); //Send web page
}

void handleReadUserPlantSelected(){
  for (int i = 0; i < SOLENOID_COUNT; i++) {
   String maxKey = "userMax[" + String(i) + "]";
   String minKey = "userMin[" + String(i) + "]";
  if (server.hasArg(maxKey)) {
   String maxArg = server.arg(maxKey);
    if (maxArg.length() > 0) {
     if (maxArg.length() <= 3 && maxArg.toInt() >= 0 && maxArg.toInt() <= 999) {
      userMax[i] = maxArg.toInt();
     } else {
      server.send(400, "text/html", "<script>alert('Invalid value for " + maxKey + "');</script>");
      return;
     }
   }
 }
 if (server.hasArg(minKey)) {
  String minArg = server.arg(minKey);
  if (minArg.length() > 0) {
    if (minArg.length() <= 3 && minArg.toInt() >= 0 && minArg.toInt() <= 999) {
      userMin[i] = minArg.toInt();
    } else {
      server.send(400, "text/html", "<script>alert('Invalid value for " + minKey + "');</script>");
      return;
    }
   }
  }
 }
  // Afficher les valeurs des seuils définis par l'utilisateur sur la console série
  Serial.println("Seuils d'humidité du sol définis par l'utilisateur :");
  for (int i = 0; i < SOLENOID_COUNT; i++) {
    Serial.print("Capteur "); Serial.print(i); Serial.print(": Max = "); Serial.print(userMax[i]); Serial.print(", Min = "); Serial.println(userMin[i]);
  }
  
  // Enregistrer les valeurs dans la mémoire EEPROM
  saveConfiguration();
  // Rediriger vers la page d'accueil
  server.sendHeader("Location","/");
  server.send(303);
}
  
void handleSendata(){
  String xml = "<?xml version=\"1.0\"?>";
  xml += "<data>";
  for(int mux = 0; mux < MUX_COUNT; mux++){
    xml += "<mux id=\"" + String(mux) + "\">";
    xml += "<sensor id=\"" + String(mux) + "\">" + String(sensorValuesTotal[mux]) + "</sensor>";
    xml += "<solenoid id=\"" + String(mux) + "\">" + String(solenoidStatus[mux]) + "</solenoid>";
    xml += "<userMax id=\"" + String(mux) +"\">" + String(userMax[mux]) + "</userMax>";
	  xml += "<userMin id=\"" + String(mux) +"\">" + String(userMin[mux]) + "</userMin>";
    xml += "</mux>"; 
  }
  xml += "<pump>" + String(pumpStatus) + "</pump>";
  xml += "</data>";
  server.send(200, "text/xml", xml);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// Définir la fonction à exécuter à chaque intervalle de ticker
void tickFunction() {
  readSensors(); // Read sensor values
  controlSolenoids(); // Activate or deactivate solenoids based on sensor values
}

// Créer un objet Ticker
Ticker ticker;

void setup() {
  Serial.begin(115200);
  while (!Serial);
    if (!mcpSolenoid.begin_I2C(0x20)) {
   Serial.println("Error.");
   while (1);
 }

  if (!mcpMuxChip.begin_I2C(0x21)) {
   Serial.println("Error.");
   while (1);
 }
  
  // Set MCP23017 #1 pins as outputs
  for (int i = 0; i < SOLENOID_COUNT; i++) {
    mcpSolenoid.pinMode(SOLENOID_PINS[i], OUTPUT);
  }
  
  // Set MCP23017 #2 pins as outputs
  for (int i = 0; i < MUX_COUNT; i++) {
    mcpMuxChip.pinMode(MUX_PINS[i], OUTPUT);
    mcpMuxChip.digitalWrite(MUX_PINS[i], HIGH);
  }
  
  pinMode(S0, OUTPUT); // Set D0 pin as output
  pinMode(S1, OUTPUT); // Set D3 pin as output
  pinMode(S2, OUTPUT); // Set D7 pin as output
  pinMode(S3, OUTPUT); // Set D8 pin as output
  pinMode(MUX_A, OUTPUT); // Set D4 pin as output
  pinMode(MUX_B, OUTPUT); // Set D5 pin as output
  pinMode(PUMP_PIN, OUTPUT); // Set pump pin as output
   // Set pump to initial state (off)
  digitalWrite(PUMP_PIN, LOW);
  pumpStatus = false;

  Serial.println("");
  delay(1000);

 Serial.println("Mounting FS...");
 if (!SPIFFS.begin()) {
   Serial.println("Failed to mount file system");
   return;
 }

  loadConfiguration();
  detectSensors();

  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Yourconnected...yeey :)"); 
  server.on("/", handleRoot);
  //server.on("/pump", handlePump);
  server.on("/get",handleReadUserPlantSelected);
  server.on("/sendata", handleSendata);
  server.onNotFound(handleNotFound);   /* page not found */

  server.begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());
  ticker.attach(tickerInterval, tickFunction);
}

void loop() {
  server.handleClient();
}
