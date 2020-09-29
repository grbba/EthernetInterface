/*
 *  © 2020, Chris Harlow. All rights reserved.
 *  
 *  This is a basic, no frills CVreader example of a DCC++ compatible setup.
 *  There are more advanced examples in the examples folder i
 */
#include "Arduino.h"
// #include "DCCEX.h"
// #include "EthernetInterface.h"

// EthernetInterface network;

#include "Singelton.h"
#include "NetworkInterface.h"

// DCCEXParser  serialParser;

NetworkInterface *nm = Singleton<NetworkInterface>::get();
// NetworkInterface *nT = new NetworkInterface(); // Singleton<NetworkInterface>::get();

void setup()
{

  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. just in case
  }
  Serial.println("Starting ...");
  
  // DCC::begin(STANDARD_MOTOR_SHIELD);

  //nm->setup(WIFI, TCP, 8888);                            // setup
  nm->setup(ETHERNET, TCP, 8888);                     // setup

  Serial.println("\nReady for DCC Commands ...");
}

void loop()
{
  // DCC::loop();
  
  nm->loop();

  // serialParser.loop(Serial);
}