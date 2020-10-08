/*
 *  © 2020, Chris Harlow. All rights reserved.
 *  
 *  This is a basic, no frills CVreader example of a DCC++ compatible setup.
 *  There are more advanced examples in the examples folder i
 */
#include <Arduino.h>

// #include "DCCEX.h"
#include "MemoryFree.h"
#include "DIAG.h"

#include "NetworkInterface.h"

// DCCEXParser  serialParser;

void setup()
{

  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. just in case
  }
  
  // DCC::begin(STANDARD_MOTOR_SHIELD);

  DIAG(F("\nNetwork Setup In Progress ...\n"));
  NetworkInterface::setup(ETHERNET, TCP, 8888);       // specify WIFI or ETHERNET depending on if you have Wifi or an EthernetShield; Wifi has to be on Serial1 UDP or TCP for the protocol
  // NetworkInterface::setup(WIFI, MQTT, 8888);      // sending over MQTT. In this case TCP will be the default as it needs a connection oriented protocol. Enabled over Wifi or Ethernet
  // NetworkInterface::setup(WIFI, UDP, 8888);    // Setup without port will use the by default port 2560
  // NetworkInterface::setup(WIFI);               // setup without port and protocol will use by default TCP on port 2560 
  // NetworkInterface::setup();                   // all defaults ETHERNET, TCP on port 2560
  DIAG(F("\nNetwork Setup done ..."));
  
  
  DIAG(F("\nFree RAM after network init: [%d]\n"),freeMemory());
  DIAG(F("\nReady for DCC Commands ..."));
}

void loop()
{
  // DCC::loop();
  NetworkInterface::loop();

  // serialParser.loop(Serial);
}