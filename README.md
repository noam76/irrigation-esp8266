# Smart Irrigation Plant System with ESP8266
This project is a smart irrigation system that use an ESP8266 microcontroller to monitoring multiple soil humidity, multiple solenoids and control a water pump to ensure that each plants receive the right amount of water.
All data are display online in web page

## The system uses the following components:
- These libraries need to be installed: 
  - Ticker (you can use anotyher way instead ticker , example millis, don't forget to make the relative change)
  - WiFiManager
  - ArduinoJson from michel blanchon
  - Adafruit_MCP23X17
- 1 ESP8266 Nodemcu V3
- 1 Pump 12V
- 15 solenoids 5v/12V
- 15 to 60 soil humidity sensor
- 1 Analog multiplexer(MUX) 16 channel
- 2 GPIO port extended MCP32017
- 16 Analog Multiplexer 8 channel 74HC4051 (I use only the 4 first channels)

## Pins configuration
```
Pump --> D6
MCP23017 SPI connection 
Multiplexer 16 Channel
  • S0 --> D0
  • S1 --> D7
  • S3 --> D8
  • S4 --> D3
  • SIG --> A0
Multiplexer 8 Channel
  • S0 --> D4
  • S1 --> D5
  • S2 --> GND
  • Z --> each Z to Analog Input from the 16 Channel Multiplexer
  • E --> each E to MCP23017 GPIO Channel
```

## Features:
* The system can connect up to a maximum of 4 soil humidity sensors per MUX, with a total of 64 sensors.
* The system automatically detects how many sensors are connected per MUX and calculates the average humidity for each plant.
* Can connect up to a maximum of 4 soil humidity sensors per MUX, with a total of 60 sensors.
* 1 water pump that is controlled according to the solenoid open/close.
* 1 Analog MUX 16 channels to read all soil sensors connected through 16 MUX 8 channel.
* 1 MCP32017 to control the solenoid.
* 1 MCP32017 to control each 16 MUX 8 channel by enable/disable each one after the other

## Getting Started
To get started with this project, you will need to gather the necessary materials and assemble the system.

Once the system is assembled, you can upload the code to the ESP8266 microcontroller

## Usage
After upload the code, you can connect to your wifi network, it automatic scan for wifi network.

To use the smart irrigation system, you need accees to the IP address that ESP8266 receive from your router, the IP is show in the serial monitor.

Simply monitor the data and status are displayed on the web page.

Users need configure for each plant the minimum and maximum rates to control the quantity of water, you need follow the quantity of water and make change as needed.

The configuration are saving after each change a JSON file in the SPIFFS and load it in the setup.

The system will automatically control the water pump based on the status solenoids, when all is closed, the pump is trun OFF.

## Contributions
Contributions to this project are welcome! If you have any ideas or suggestions for how to improve the system, please feel free to open an issue or pull request.

### Acknowledgements
This project was inspired by the need for a more efficient and effective way to water plants. Thanks to chatgpt that helping me to improve my old code.
