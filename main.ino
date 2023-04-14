#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Adafruit_MCP23X17.h>

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

const int SENSOR_COUNT = 4; // maximum number of sensors
const int SOLENOID_COUNT = 15; // number of solenoids connected to MCP23017 #1
const int MUX_COUNT = 15; // number of MUX cd74hc4051 connected to MCP23017 #2

// Définition des broches de sortie pour chaque solénoïde
const int SOLENOID_PINS[SOLENOID_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}; // MCP23017 pins connected to solenoids
const int MUX_PINS[MUX_COUNT] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}; // MCP23017 pins connected to cd74hc4051

int sensorValuesMux[MUX_COUNT][SENSOR_COUNT]; // 2D array to store sensor values from all multiplexers
int userMax[SOLENOID_COUNT]; // Define array to store user-defined maximum values
int userMin[SOLENOID_COUNT]; // Define array to store user-defined minimum values
bool anySolenoidsActive = false; // flag to indicate if any solenoids are active

ESP8266WebServer server(80);
static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8" />
 <title>Irrigation Viewer</title>
 <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css">
 <link href='https://fonts.googleapis.com/css?family=Rock+Salt' rel='stylesheet' type='text/css'>
<style type="text/css">
/**
 * Layout
 */
body {
  background-color: #ccc;
  padding: 0% 3% 0% 3%;
  font-size: 14px;
  font-family: Lato, sans-serif;
}

.main {
  width: 80%;
  display: flex;
  flex-direction: column;
  margin-top: 75px;
}

.wrapper {
  display: flex;
  justify-content: space-between;
}

.sidebar {
  width: 15%;
  height: 25vh;
  min-height: 150px;
  position: -webkit-sticky;
  position: sticky;
  top: 20%;
  text-align: center;
  border: 3px solid purple;
  padding: 17px;
  margin-left: 5px;
}

ins {
  text-decoration: none;
}

h2 {
  margin: 0;
  padding: 0;
}

table {
  table-layout: fixed;
  width: 103%;
  border-collapse: collapse;
  border: 3px solid purple;
}

thead th, tfoot th {
  font-family: 'Rock Salt', cursive;
}

th {
  letter-spacing: 2px;
}

td {
  letter-spacing: 1px;
}

tbody td {
  text-align: center;
}

thead {
  color: white;
  text-shadow: 1px 1px 1px black;
}

tfoot {
  color: black;
  text-align: center;
}

thead th, tfoot th, tfoot td {
  background: linear-gradient(to bottom, rgba(0,0,0,0.1), rgba(0,0,0,0.5));
  border: 3px solid purple;
}

th, td {
  padding: 10px;
}

tbody tr:nth-child(odd) {
  background-color: #ffffff;
}

tbody tr:nth-child(even) {
  background-color: #EB984E;
}

/**
 * Sticky navigation
 */
.sticky {
  text-align: center;
  background-color: #ccc;
  position: -webkit-sticky;
  position: fixed;
  top: 0;
  height: 63px;
  width: 100%;
}

/**
 * Display lists on a single line.
 * @bugfix Prevent webkit from removing list semantics
 * 1. Add a non-breaking space
 * 2. Make sure it doesn't mess up the DOM flow
 */
.list-inline {
  list-style: none;
  margin-left: -0.5em;
  margin-right: -0.5em;
  padding: 0;
}

.list-inline > li {
  display: inline-block;
  margin-left: 0.5em;
  margin-right: 0.5em;
}

.list-inline > li:before {
  content: "\200B"; /* 1 */
  position: absolute; /* 2 */
}

input[type=submit]{
  background-color: #EB984E;
  border-radius: 10px;
  border: 0;
  width: 80px;
  height:30px;
  text-decoration: none;
  cursor: pointer;
}

.sidebar-send-btn {
  background-color: #EB984E;
  border-radius: 10px;
  border: 0;
  width: 80px;
  height: 30px;
  text-decoration: none;
  cursor: pointer;
}
</style>
</head>
<body onload="obtenirVariables()">
 <div class="sticky">
   <h1>Irrigation System Viewer</h1>
 </div>
 <div class="wrapper">
   <div class="sidebar">
   <h3>Pump State</h3>
   <div id="pumpstate"></div>
   <br>
   <div><i class="fa fa-refresh fa-2x fa-fw"></i></div>
  <br><br><br><br><br><br><br><br><br>

  <table>
    <tr>
      <th scope="row"><i class="fa fa-paper-plane fa-2x" aria-hidden="true"></i> Send Data</th>
      <td><button class="sidebar-send-btn" onclick="document.getElementById('myForm').submit()">Send</button></td>
    </tr>
  </table>
  </div> <!--  end sidebar -->
 <div class="main">
 <form id="myForm" action="/get">
 <table>
   <thead>
     <tr>
       <th scope="col">Plant</th>
       <th scope="col"><i class="fa fa-thermometer-three-quarters fa-2x" aria-hidden="true"></i> Humidity %</th>
       <th scope="col">Tap Status</th>
       <th scope="col">Tap Icon</th>
     <th scope="col" colspan="2">Select humidity rate</th>
     </tr>
   </thead>
   <tbody>
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 0</th>
       <td id="sensor0"></td>
       <td id="solenoidState0"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max00"></ins> <input type="text" name="S00max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min00"></ins> <input type="text" name="S00min" maxlength="4" size="4"></td>
     </tr>
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 1</th>
       <td id="sensor1"></td>
       <td id="solenoidState1"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max01"></ins> <input type="text" name="S01max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min01"></ins> <input type="text" name="S01min" maxlength="4" size="4"></td>
     </tr>
 
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 2</th>
       <td id="sensor2"></td>
       <td id="solenoidState2"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max02"></ins> <input type="text" name="S02max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min02"></ins> <input type="text" name="S02min" maxlength="4" size="4"></td>
     </tr>
 
     <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 3</th>
       <td id="sensor3"></td>
       <td id="solenoidState3"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max03"></ins> <input type="text" name="S03max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min03"></ins> <input type="text" name="S03min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 4</th>
       <td id="sensor4"></td>
       <td id="solenoidState4"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max04"></ins> <input type="text" name="S04max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min04"></ins> <input type="text" name="S04min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 5</th>
       <td id="sensor5"></td>
       <td id="solenoidState5"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max05"></ins> <input type="text" name="S05max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min05"></ins> <input type="text" name="S05min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 6</th>
       <td id="sensor6"></td>
       <td id="solenoidState6"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max06"></ins> <input type="text" name="S06max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min06"></ins> <input type="text" name="S06min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 7</th>
       <td id="sensor7"></td>
       <td id="solenoidState7"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max07"></ins> <input type="text" name="S07max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min07"></ins> <input type="text" name="S07min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 8</th>
       <td id="sensor8"></td>
       <td id="solenoidState8"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max08"></ins> <input type="text" name="S08max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min08"></ins> <input type="text" name="S08min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 9</th>
       <td id="sensor9"></td>
       <td id="solenoidState9"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max09"></ins> <input type="text" name="S09max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min09"></ins> <input type="text" name="S09min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 10</th>
       <td id="sensor10"></td>
       <td id="solenoidState10"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max10"></ins> <input type="text" name="S10max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min10"></ins> <input type="text" name="S10min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 11</th>
       <td id="sensor11"></td>
       <td id="solenoidState11"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max11"></ins> <input type="text" name="S11max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min11"></ins> <input type="text" name="S11min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 12</th>
       <td id="sensor12"></td>
       <td id="solenoidState12"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max12"></ins> <input type="text" name="S12max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min12"></ins> <input type="text" name="S12min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 13</th>
       <td id="sensor13"></td>
       <td id="solenoidState13"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max13"></ins> <input type="text" name="S13max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min13"></ins> <input type="text" name="S13min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 14</th>
       <td id="sensor14"></td>
       <td id="solenoidState14"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max14"></ins> <input type="text" name="S14max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min14"></ins> <input type="text" name="S14min" maxlength="4" size="4"></td>
     </tr>
  
  <tr>
       <th scope="row"><i class="fa fa-tree fa-2x" aria-hidden="true"></i> Sensor 15</th>
       <td id="sensor15"></td>
       <td id="solenoidState15"></td>
       <td><i class="fa fa-refresh fa-2x fa-fw"></i></td>
     <td>Max: <ins id="max15"></ins> <input type="text" name="S15max" maxlength="4" size="4"></td>
     <td>Min: <ins id="min15"></ins> <input type="text" name="S15min" maxlength="4" size="4"></td>
     </tr>
   </tbody>
  </table>
  <br>
  </form> 
  </div> <!-- end main -->
 </div> <!-- end wrapper -->
<script type="text/javascript">
    function obtenirVariables(){
    var uniqueURL = "/reqEtatVariables" + "?aleatoire=" + Math.trunc(Math.random() * 1000000);
    var request = new XMLHttpRequest(); // http://www.toutjavascript.com/reference/ref-xmlhttprequest.php
    // la fonction à appeler lors d'un changement d'avancement de la requête AJAX
    request.onreadystatechange = function()
    {
     if (request.readyState == 4) {
      // Indicateur de l'avancement de l'appel AJAX == 4 => Données complètement accessibles 
      if (request.status == 200) {
     document.getElementById("pumpstate").innerHTML = this.responseXML.getElementsByTagName('pumpstate')[0].childNodes[0].nodeValue;
       if (document.getElementById("pumpstate").innerHTML === "ON"){
        document.getElementById("pumpstate").style.color = "greenyellow";
       }
       else {
        document.getElementById("pumpstate").style.color = "#ff0000";
      }
     
     document.getElementById("sensor0").innerHTML = this.responseXML.getElementsByTagName('sensor0')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor1").innerHTML = this.responseXML.getElementsByTagName('sensor1')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor2").innerHTML = this.responseXML.getElementsByTagName('sensor2')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor3").innerHTML = this.responseXML.getElementsByTagName('sensor3')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor4").innerHTML = this.responseXML.getElementsByTagName('sensor4')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor5").innerHTML = this.responseXML.getElementsByTagName('sensor5')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor6").innerHTML = this.responseXML.getElementsByTagName('sensor6')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor7").innerHTML = this.responseXML.getElementsByTagName('sensor7')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor8").innerHTML = this.responseXML.getElementsByTagName('sensor8')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor9").innerHTML = this.responseXML.getElementsByTagName('sensor9')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor10").innerHTML = this.responseXML.getElementsByTagName('sensor10')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor11").innerHTML = this.responseXML.getElementsByTagName('sensor11')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor12").innerHTML = this.responseXML.getElementsByTagName('sensor12')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor13").innerHTML = this.responseXML.getElementsByTagName('sensor13')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor14").innerHTML = this.responseXML.getElementsByTagName('sensor14')[0].childNodes[0].nodeValue + "%";
     document.getElementById("sensor15").innerHTML = this.responseXML.getElementsByTagName('sensor15')[0].childNodes[0].nodeValue + "%";
     
     document.getElementById("solenoidState0").innerHTML = this.responseXML.getElementsByTagName('solenoidState0')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState0").innerHTML === "ON"){
        document.getElementById("solenoidState0").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState0").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState1").innerHTML = this.responseXML.getElementsByTagName('solenoidState1')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState1").innerHTML === "ON"){
        document.getElementById("solenoidState1").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState1").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState2").innerHTML = this.responseXML.getElementsByTagName('solenoidState2')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState2").innerHTML === "ON"){
        document.getElementById("solenoidState2").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState2").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState3").innerHTML = this.responseXML.getElementsByTagName('solenoidState3')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState3").innerHTML === "ON"){
        document.getElementById("solenoidState3").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState3").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState4").innerHTML = this.responseXML.getElementsByTagName('solenoidState4')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState4").innerHTML === "ON"){
        document.getElementById("solenoidState4").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState4").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState5").innerHTML = this.responseXML.getElementsByTagName('solenoidState5')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState5").innerHTML === "ON"){
        document.getElementById("solenoidState5").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState5").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState6").innerHTML = this.responseXML.getElementsByTagName('solenoidState6')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState6").innerHTML === "ON"){
        document.getElementById("solenoidState6").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState6").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState7").innerHTML = this.responseXML.getElementsByTagName('solenoidState7')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState7").innerHTML === "ON"){
        document.getElementById("solenoidState7").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState7").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState8").innerHTML = this.responseXML.getElementsByTagName('solenoidState8')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState8").innerHTML === "ON"){
        document.getElementById("solenoidState8").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState8").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState9").innerHTML = this.responseXML.getElementsByTagName('solenoidState9')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState9").innerHTML === "ON"){
        document.getElementById("solenoidState9").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState9").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState10").innerHTML = this.responseXML.getElementsByTagName('solenoidState10')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState10").innerHTML === "ON"){
        document.getElementById("solenoidState10").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState10").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState11").innerHTML = this.responseXML.getElementsByTagName('solenoidState11')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState11").innerHTML === "ON"){
        document.getElementById("solenoidState11").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState11").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState12").innerHTML = this.responseXML.getElementsByTagName('solenoidState12')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState12").innerHTML === "ON"){
        document.getElementById("solenoidState12").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState12").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState13").innerHTML = this.responseXML.getElementsByTagName('solenoidState13')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState13").innerHTML === "ON"){
        document.getElementById("solenoidState13").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState13").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState14").innerHTML = this.responseXML.getElementsByTagName('solenoidState14')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState14").innerHTML === "ON"){
        document.getElementById("solenoidState14").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState14").style.color = "#ff0000";
      }
     
     document.getElementById("solenoidState15").innerHTML = this.responseXML.getElementsByTagName('solenoidState15')[0].childNodes[0].nodeValue;
        if (document.getElementById("solenoidState15").innerHTML === "ON"){
        document.getElementById("solenoidState15").style.color = "greenyellow";
       }
       else {
        document.getElementById("solenoidState15").style.color = "#ff0000";
      }

     document.getElementById("max00").innerHTML = this.responseXML.getElementsByTagName('max00')[0].childNodes[0].nodeValue;
     document.getElementById("min00").innerHTML = this.responseXML.getElementsByTagName('min00')[0].childNodes[0].nodeValue;
     document.getElementById("max01").innerHTML = this.responseXML.getElementsByTagName('max01')[0].childNodes[0].nodeValue;
     document.getElementById("max02").innerHTML = this.responseXML.getElementsByTagName('max02')[0].childNodes[0].nodeValue;
     document.getElementById("max03").innerHTML = this.responseXML.getElementsByTagName('max03')[0].childNodes[0].nodeValue;
     document.getElementById("max04").innerHTML = this.responseXML.getElementsByTagName('max04')[0].childNodes[0].nodeValue;
     document.getElementById("max05").innerHTML = this.responseXML.getElementsByTagName('max05')[0].childNodes[0].nodeValue;
     document.getElementById("max06").innerHTML = this.responseXML.getElementsByTagName('max06')[0].childNodes[0].nodeValue;
     document.getElementById("max07").innerHTML = this.responseXML.getElementsByTagName('max07')[0].childNodes[0].nodeValue;
     document.getElementById("max08").innerHTML = this.responseXML.getElementsByTagName('max08')[0].childNodes[0].nodeValue;
     document.getElementById("max09").innerHTML = this.responseXML.getElementsByTagName('max09')[0].childNodes[0].nodeValue;
     document.getElementById("max10").innerHTML = this.responseXML.getElementsByTagName('max10')[0].childNodes[0].nodeValue;
     document.getElementById("max11").innerHTML = this.responseXML.getElementsByTagName('max11')[0].childNodes[0].nodeValue;
     document.getElementById("max12").innerHTML = this.responseXML.getElementsByTagName('max12')[0].childNodes[0].nodeValue;
     document.getElementById("max13").innerHTML = this.responseXML.getElementsByTagName('max13')[0].childNodes[0].nodeValue;
     document.getElementById("max14").innerHTML = this.responseXML.getElementsByTagName('max14')[0].childNodes[0].nodeValue;
     document.getElementById("max15").innerHTML = this.responseXML.getElementsByTagName('max15')[0].childNodes[0].nodeValue;
     document.getElementById("min01").innerHTML = this.responseXML.getElementsByTagName('min01')[0].childNodes[0].nodeValue;
     document.getElementById("min02").innerHTML = this.responseXML.getElementsByTagName('min02')[0].childNodes[0].nodeValue;
     document.getElementById("min03").innerHTML = this.responseXML.getElementsByTagName('min03')[0].childNodes[0].nodeValue;
     document.getElementById("min04").innerHTML = this.responseXML.getElementsByTagName('min04')[0].childNodes[0].nodeValue;
     document.getElementById("min05").innerHTML = this.responseXML.getElementsByTagName('min05')[0].childNodes[0].nodeValue;
     document.getElementById("min06").innerHTML = this.responseXML.getElementsByTagName('min06')[0].childNodes[0].nodeValue;
     document.getElementById("min07").innerHTML = this.responseXML.getElementsByTagName('min07')[0].childNodes[0].nodeValue;
     document.getElementById("min08").innerHTML = this.responseXML.getElementsByTagName('min08')[0].childNodes[0].nodeValue;
     document.getElementById("min09").innerHTML = this.responseXML.getElementsByTagName('min09')[0].childNodes[0].nodeValue;
     document.getElementById("min10").innerHTML = this.responseXML.getElementsByTagName('min10')[0].childNodes[0].nodeValue;
     document.getElementById("min11").innerHTML = this.responseXML.getElementsByTagName('min11')[0].childNodes[0].nodeValue;
     document.getElementById("min12").innerHTML = this.responseXML.getElementsByTagName('min12')[0].childNodes[0].nodeValue;
     document.getElementById("min13").innerHTML = this.responseXML.getElementsByTagName('min13')[0].childNodes[0].nodeValue;
     document.getElementById("min14").innerHTML = this.responseXML.getElementsByTagName('min14')[0].childNodes[0].nodeValue;
     document.getElementById("min15").innerHTML = this.responseXML.getElementsByTagName('min15')[0].childNodes[0].nodeValue;
       }
     }
    }
    request.open("GET", uniqueURL , true); // ici on envoie la requête GET sur l'URL /reqEtatVariables
    request.send(null);
    setTimeout("obtenirVariables()", 1000); // on rappelle obtenirVariables()
   }
     function sendForm() {
	 document.getElementById("myForm").submit();
	}

  document.getElementById("myForm").addEventListener("submit", function(event) {
  event.preventDefault(); // prevent the form from submitting
  displayFormValues(); // call the function to display the form values
  });
 </script>
</body>
</html>
)rawliteral";


void readSensors() {
  for (int mux = 0; mux < MUX_COUNT; mux++) { // Loop through all multiplexers
    mcpMuxChip.digitalWrite(MUX_PINS[mux], LOW); // Enable current multiplexer

    // Detect number of sensors connected to current multiplexer
    int numSensors = 0;
    for (int channel = 0; channel < SENSOR_COUNT; channel++) {
      digitalWrite(S0, channel & 0x01); // set S0
      digitalWrite(S1, channel & 0x02); // set S1
      digitalWrite(S2, channel & 0x04); // set S2
      digitalWrite(S3, channel & 0x08); // set S3
      digitalWrite(MUX_A, channel & 0x10); // set MUX_A
      digitalWrite(MUX_B, channel & 0x20); // set MUX_B
      int sensorValue = analogRead(ANALOG_PIN);
      if (sensorValue > 0) { // if a sensor is connected to this channel
        numSensors++; // increment the number of sensors connected to this multiplexer
      }
    }

    // Read sensor values and store them in corresponding variables
    for (int channel = 0; channel < numSensors; channel++) {
      digitalWrite(S0, channel & 0x01); // set S0
      digitalWrite(S1, channel & 0x02); // set S1
      digitalWrite(S2, channel & 0x04); // set S2
      digitalWrite(S3, channel & 0x08); // set S3
      digitalWrite(MUX_A, channel & 0x10); // set MUX_A
      digitalWrite(MUX_B, channel & 0x20); // set MUX_B
      sensorValuesMux[mux][channel] = analogRead(ANALOG_PIN);
    }

    mcpMuxChip.digitalWrite(MUX_PINS[mux], HIGH); // Disable current multiplexer
  }
}

void controlSolenoids() {
  for (int solenoid = 0; solenoid < SOLENOID_COUNT; solenoid++) {
    bool solenoidActive = false;
    for (int mux = 0; mux < MUX_COUNT; mux++) {
      if (sensorValuesMux[mux][solenoid] > sensorValuesMux[MUX_COUNT][SENSOR_COUNT]) {
        solenoidActive = true;
        anySolenoidsActive = true;
        break;
      }
    }
    mcpSolenoid.digitalWrite(SOLENOID_PINS[solenoid], solenoidActive);
  }

  if (anySolenoidsActive) {
    digitalWrite(PUMP_PIN, HIGH);
  } else {
    digitalWrite(PUMP_PIN, LOW);
  }

  anySolenoidsActive = false;
}


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
  if (maxArray.size() != SOLENOID_COUNT) {
    Serial.println(F("Invalid userMax array size"));
    return;
  }
  for (int i = 0; i < SOLENOID_COUNT; i++) {
    userMax[i] = maxArray[i];
  }
  JsonArray minArray = doc["userMin"];
  if (minArray.size() != SOLENOID_COUNT) {
    Serial.println(F("Invalid userMin array size"));
    return;
  }
  for (int i = 0; i < SOLENOID_COUNT; i++) {
    userMin[i] = minArray[i];
  }

  // Print the loaded user-defined maximum and minimum values for each sensor to the serial monitor
  Serial.println("Loaded user-defined soil humidity thresholds:");
  for (int i = 0; i < SOLENOID_COUNT; i++) {
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


void handleRoot(){
  server.send_P(200, "text/html", INDEX_HTML); //Send web page
}

void handleReadUserPlantSelected(){
for (int i = 0; i < SOLENOID_COUNT; i++) {
  String maxKey = "S" + String(i, 2) + "max";
  String minKey = "S" + String(i, 2) + "min";
  if (server.hasArg(maxKey)) {
    String maxArg = server.arg(maxKey);
    if (maxArg.length() > 0) {
      userMax[i] = maxArg.toInt();
    }
  }
  if (server.hasArg(minKey)) {
    String minArg = server.arg(minKey);
    if (minArg.length() > 0) {
      userMin[i] = minArg.toInt();
    }
  }
}
  
  // Print the user-defined maximum and minimum values for each sensor to the serial monitor
  Serial.println("User-defined Soil Humidity Thresholds:");
  for (int i = 0; i < SOLENOID_COUNT; i++) {
    Serial.print("Sensor "); Serial.print(i); Serial.print(": Max = "); Serial.print(userMax[i]); Serial.print(", Min = "); Serial.println(userMin[i]);
  }
  
 saveConfiguration();

  // Redirect back to the home page
  server.sendHeader("Location","/");
  server.send(303);
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
  }
  
  pinMode(S0, OUTPUT); // Set D0 pin as output
  pinMode(S1, OUTPUT); // Set D3 pin as output
  pinMode(S2, OUTPUT); // Set D7 pin as output
  pinMode(S3, OUTPUT); // Set D8 pin as output
  pinMode(MUX_A, OUTPUT); // Set D4 pin as output
  pinMode(MUX_B, OUTPUT); // Set D5 pin as output
  pinMode(PUMP_PIN, OUTPUT); // Set pump pin as output

  Serial.println("");
  delay(1000);

  Serial.println("Mounting FS...");
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }

  loadConfiguration();
  
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Yourconnected...yeey :)"); 
  server.on("/", handleRoot);
  //server.on("/reqEtatVariables",handleXML);
  server.on("/get",handleReadUserPlantSelected);
  server.onNotFound(handleNotFound);   /* page not found */

  server.begin();
  Serial.println("HTTP server started");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();

  digitalWrite(S0, LOW); // set S0
  digitalWrite(S1, LOW); // set S1
  digitalWrite(S2, LOW); // set S2
  digitalWrite(S3, LOW); // set S3
  delay(850);

  Serial.println(analogRead(ANALOG_PIN));
  readSensors(); // Read sensor values
  //controlSolenoids(); // Activate or deactivate solenoids based on sensor values
}
