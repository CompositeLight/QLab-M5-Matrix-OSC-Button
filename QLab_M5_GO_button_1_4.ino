/* Large sections of this code (the good bits) have been borrowed/stolen/magpied from the work of genius Joseph Adams */

#include <M5Atom.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <PinButton.h>
#include <stdint.h>
#include <Arduino.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#define DATA_PIN_LED 27
using namespace std;


// --------------------------------------------------------------------

/* USER CONFIG VARIABLES
    Change the following variables before compiling and sending the code to your device.
*/

//Wifi SSID and password
const char * networkSSID = "My Wifi Network";
const char * networkPass = "MyWiFiPaSsWoRd";

// OSC Settings
const IPAddress outIp(192,168,1,100);        // Target IP of your QLab machine
const unsigned int outPort = 53000;          // remote port to send OSC commands to (default 53000 for QLab)
const unsigned int localPort = 53001;        // local port to listen for OSC replies (default 53001 for QLab)

// QLab Settings
const char * oscCommand = "/go"; // message to send to QLab eg. "/go"
int qlabCheckInterval = 30; // Number of seconds between QLab 'thump' checks


// --------------------------------------------------------------------


//General Variables
bool networkConnected = false;    // State of wifi connection
bool qlabConnected = false;        // Whether a thump was returned from QLab
uint8_t FSM = 0;

int refreshCounter = 0;
int refreshInterval = (qlabCheckInterval * 20);


OSCErrorCode error;

//M5 variables
PinButton btnAction(39); //the "Action" button on the device


// default color values
int GRB_COLOR_WHITE = 0xffffff;
int GRB_COLOR_BLACK = 0x000000;
int GRB_COLOR_RED = 0x00ff00;
int GRB_COLOR_ORANGE = 0xa5ff00;
int GRB_COLOR_YELLOW = 0xffff00;
int GRB_COLOR_GREEN = 0xff0000;
int GRB_COLOR_BLUE = 0x0000ff;
int GRB_COLOR_PURPLE = 0x008080;

int numbercolor = GRB_COLOR_ORANGE;
int standbycolor[] = {GRB_COLOR_RED, numbercolor};
int gocolor[] = {GRB_COLOR_GREEN, numbercolor};
int qlaberrorcolour[] = {GRB_COLOR_RED, GRB_COLOR_BLACK};
int alloffcolor[] = {GRB_COLOR_BLACK, GRB_COLOR_BLACK};
int wificolor[] = {GRB_COLOR_BLUE, GRB_COLOR_BLACK};

int currentBrightness = 20; // DO NOT GO ABOVE 20 ON AN M5!!!

//this is the array that stores the LED looks
int number[19][25] = {{
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0, 0
  },
  { 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1
  },
  { 1, 1, 1, 1, 1,
    1, 0, 0, 0, 1,
    1, 0, 1, 0, 1,
    1, 0, 0, 0, 1,
    1, 1, 1, 1, 1
  },
  { 0, 1, 1, 1, 0,
    1, 0, 1, 0, 1,
    1, 1, 0, 1, 1,
    1, 0, 1, 0, 1,
    0, 1, 1, 1, 0
  },


  
};


WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP


void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("Network connected!");
      // Serial.println(String(WiFi.localIP()));
      Serial.println(WiFi.localIP().toString());
      networkConnected = true;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("Network connection lost!");
      networkConnected = false;
      break;
  }
}

//---------------------------------------------------------------
//HERE IS THE MAIN LED DRAWING ROUTINE aka drawNumber
void drawNumber(int arr[], int colors[])
{
  for (int i = 0; i < 25; i++)
  {
    M5.dis.drawpix(i, colors[arr[i]]);
  }
}
//---------------------------------------------------------------


//Change colour of LEDs

void greensquare(){
  M5.dis.clear();
  drawNumber(number[0], gocolor);
}

void redsquare(){
  M5.dis.clear();
  drawNumber(number[0], standbycolor);
}

void wifiErrorSquare(){
  M5.dis.clear();
  drawNumber(number[3], wificolor);
}


void qlabErrorSquare(){
  M5.dis.clear();
  drawNumber(number[3], qlaberrorcolour);
}



void thumpRX(OSCMessage &rxmsg, int addrOffset) {
  qlabConnected = true; 
  Serial.println("QLab: Thump!");
}


// Send QLab thump OSC
void qlabThump(){
    // First send a THUMP to QLab to generate a reply
    qlabConnected = false;
    OSCMessage msg("/thump");
    msg.add("");
    Udp.beginPacket(outIp, outPort);
    msg.send(Udp);
    Udp.endPacket();
    msg.empty();
    delay (100);
    Serial.println("THUMP SENT");

    // Now watch for a THUMP response to confirm QLab is there
    OSCMessage rxmsg;
    int size = Udp.parsePacket();
    if (size > 0) {
      IPAddress remoteIp = Udp.remoteIP();
      Serial.println("Reply was received from: " +  remoteIp.toString());
      while (size--) {
        rxmsg.fill(Udp.read());
      }
      if (!rxmsg.hasError()) {
        rxmsg.route("/*/*/*/thump", thumpRX);
      } else {
        error = rxmsg.getError();
        Serial.print("error: ");
        Serial.println(error);
      }
    }

}



void connectToNetwork() {
  Serial.println("");
  Serial.println("Connecting to SSID: " + String(networkSSID));

  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);

  WiFi.mode(WIFI_STA); //station
  WiFi.setSleep(false);

  WiFi.begin(networkSSID, networkPass);
}



// --------------------------------------------------------------------------------------------------------------------
// Setup is the pre-loop running program

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Initializing M5-Atom.");

  M5.begin(true, false, true);
  delay(50);
  M5.dis.drawpix(0, 0xf00000);

  // blanks out the screen
  // drawNumber(number[17], alloffcolor);
  wifiErrorSquare();
  delay(100); //wait 100ms before moving on

  connectToNetwork(); //starts Wifi connection
  delay(5000);
  while (!networkConnected) {
    connectToNetwork(); //starts Wifi connection
    delay(5000);
  }
  // UDP connect for OSC
  Udp.begin(localPort);
  // Flash screen if connected to wifi.
  drawNumber(number[0], alloffcolor);
  delay(100);
  drawNumber(number[2], wificolor);
  delay(400);
  drawNumber(number[0], alloffcolor);
  delay(200);
  drawNumber(number[2], wificolor);
  delay(400);
  drawNumber(number[0], alloffcolor);
  delay(200);
  drawNumber(number[2], wificolor);
  delay(400);
  drawNumber(number[0], alloffcolor);
  delay(100);
  greensquare();
  qlabThump();
  delay(200);
}
// --------------------------------------------------------------------------------------------------------------------

// --------------------------------------------------------------------------------------------------------------------
// This is the main program loop
void loop()
{
  if (networkConnected && qlabConnected) {
    if (M5.Btn.wasPressed()) {
      redsquare();
      
      // First send attempt    
      OSCMessage msg(oscCommand); 
      msg.add("");
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      delay (100);
      
      // Second send attempt
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      
      delay (200);
      Serial.println("OSC SENT: /go");
      Serial.println("--------");
      
      
      qlabThump(); // Send heartbeat to QLab to check it's still there
      
      
    } else if (refreshCounter > refreshInterval) { // Check for QLab every 30 seconds
      refreshCounter = 0;
      qlabThump(); // Send heartbeat to QLab to check it's still there
    }
  
  } else {

    if (!qlabConnected){
      // means qlab not responding to thump heartbeat
      qlabErrorSquare();
      qlabThump(); // Send heartbeat to QLab to check it's still there
      delay(200);
    }

    if (!networkConnected){
      // Lost Network Connection
      wifiErrorSquare();
      connectToNetwork(); //starts Wifi connection
      while (!networkConnected) {
        delay(200);
      }
      // UDP connect for OSC
      Udp.begin(localPort);
      greensquare();
    }
  }
  
  delay(50);
  refreshCounter++;
  greensquare();
  M5.update();

}
// --------------------------------------------------------------------------------------------------------------------
