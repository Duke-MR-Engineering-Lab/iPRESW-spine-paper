/* Controls the 4-channel wireless battery box for the iPRES-W spine array.
 * The wireless module (ESP8266) connects to the defined WiFi network and initiates a telnet connection.
 * Commands are sent from a computer to the wireless module to set the PWM duty cycle of each GPIO pin.
 * 
 * Wrttin by: Jonathan Cuthbertson 
*/

#include <ESP8266WiFi.h>  // wireless module command library
#include <Arduino.h>  // basic Arduino command library 

// Network Setup Info
const char *ssid = "XXXXXXXXXX";  // add name of network
const char *password = "XXXXXXXXXX";  // add network password

// Connection Setup
#define MAX_SRV_CLIENTS 1 // max allowed connections to wireless module                                        
WiFiServer server(23);  // telnet server
WiFiClient serverClients[MAX_SRV_CLIENTS];  // defines client list

// Defined Variables
int PWMValues[4] = {510, 510, 510, 510};  // PWM duty cycles between 0%-100% mapped to bit resolution of 0-1023 (i.e., 510 = 0A, >510 = 0A to 1.3A, <510 = -1.3A to 0A)
int channelPin[4] = {3, 5, 4, 12}; //GPIO pins for channels 1 - 4
char transmitChar;  // characters received from client
String transmitString;  // stores above characters as string
int transmitInt;  // stores above characters as integer

void setup() {
  analogWriteFreq(4000);  // PWM frequency

  // Sets the GPIO function
  pinMode(channelPin[0], FUNCTION_3); // changes original pin function

  for (int j = 0; j <= 3; j++) {  // sets the GPIOs to outputs
    pinMode(channelPin[j], OUTPUT);
  }

  // Starts the PWM signals
  for (int j = 0; j <= 3; j++) {
    analogWrite(channelPin[j], PWMValues[j]);
  }

  // Begins the Wifi connection
  WiFi.mode(WIFI_STA);  // station mode
  WiFi.begin(ssid, password); // connects to network
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++) { // waits for connection
    delay(500);
  }

  // Begins server connection
  server.begin();
  server.setNoDelay(true);
  delay(500);
}

void loop() {
  uint8_t i;

  // Looks for new client
  if (server.hasClient()) {
    for (i = 0; i < MAX_SRV_CLIENTS; i++) {
      if (!serverClients[i] || !serverClients[i].connected()) {
        if (serverClients[i]) {
          serverClients[i].stop();
        }
        serverClients[i] = server.available();
        break;
      }
    }
    if (i == MAX_SRV_CLIENTS) {
      WiFiClient serverClient = server.available();
      serverClient.stop();
    }
  }

  // Looks for data from client
  for (i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i] && serverClients[i].connected()) {
      if (serverClients[i].available()) {
        while (serverClients[i].available()) {
          transmitChar = serverClients[i].read(); // reads data
          transmitString += transmitChar; // converts data to a string
        }
      }
    }
  } 

  // Commands are sent in the form XAAAA, where "X" is the channel number (1 - 4) and "AAAA" is the mapped PWM value (0 - 1023)
  // For example, a command of "20700" sets the duty cycle of channel 2 to 68%, which corresponds to ~480 mA
  transmitString.trim();
  if (transmitString.length() > 0) {  // ignores data sent in the incorrect form
    int transmitInt = transmitString.toInt(); // converts data to integer
    if (transmitInt >= 10000 && transmitInt <= 11023) { // channel 1
      PWMValues[0] = transmitInt - 10000;
      analogWrite(3, PWMValues[0]);
    }
    if (transmitInt >= 20000 && transmitInt <= 21023) { // channel 2
      PWMValues[1] = transmitInt - 20000;
      analogWrite(5, PWMValues[1]);
    }
    if (transmitInt >= 30000 && transmitInt <= 31023) { // channel 3
      PWMValues[2] = transmitInt - 30000;
      analogWrite(4, PWMValues[2]);
    }
    if (transmitInt >= 40000 && transmitInt <= 41023) { // channel 4
      PWMValues[3] = transmitInt - 40000;
      analogWrite(12, PWMValues[3]);
    }
    transmitString = "";  // clears previous command
  }
}  
