/*
 *  © 2020, Gregor Baues, Chris Harlow. All rights reserved.
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

/**
 * @brief User define callback for HTTP requests. The Network interface will provide for each http request a parsed request object 
 * and the client who send the request are provided. Its up to the user to use the req as he sees fits. Below is just a scaffold to
 * demonstrate the workings.
 * 
 * @param req     Parsed request object
 * @param client  Originator of the request to reply to 
 */


void httpRequestHandler(ParsedRequest *req, Client* client) {
  DIAG(F("\nParsed Request:"));
  DIAG(F("\nMethod:         [%s]"), req->method);
  DIAG(F("\nURI:            [%s]"), req->uri);
  DIAG(F("\nHTTP version:   [%s]"), req->version);
  DIAG(F("\nParameter count:[%d]\n"), *req->paramCount);
  
  // result = doSomething(); // obtain result to be send back; fully prepare the serialized HTTP response!

  // client->write(result);
}

NetworkInterface wifi1;
// NetworkInterface wifi2;
NetworkInterface eth1;
// NetworkInterface eth2;

void setup()
{

  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect. just in case
  }

  // DCC::begin(STANDARD_MOTOR_SHIELD);
  DIAG(F("\nFree RAM before network init: [%d]\n"),freeMemory());
  DIAG(F("\nNetwork Setup In Progress ...\n"));

  wifi1.setup(WIFI);                                           // There will be only one WIFI transport allowed for now. Its not clear if the underlying AT library actually supports 
  // wifi2.setup(WIFI,TCP,3000);                               // multiple ports. Current guess is that everytime WiFi Init is called all is reset and there no port muktiplexing implemented

  eth1.setup(ETHERNET, TCP, 8888); 
  // eth1.setHttpCallback(httpRequestHandler); 
  // eth2.setup(ETHERNET, TCP); 


  // WIFI, TCP on Port 2560, Wifi (ssid/password) has been configured permanetly already on the esp. If
  // the connection fails will go into AP mode 

  // wifi.setup(WIFI);   

  // New connection on known ssid / password combo / port can be added as a last parameter other wise the default of 2560
  // will be used. If it passes the connection will be stored as permanent default. If fails will go into AP mode.                
  
  // wifi.setup(WIFI, TCP, F(WIFI_SSID), F(WIFI_PASSWORD), F(WIFI_HOSTNAME));
  // wifi.setup(WIFI, TCP, F(WIFI_SSID), F(WIFI_PASSWORD), F(WIFI_HOSTNAME, 2323);

  // <obj>.setup(ETHERNET, TCP, 8888);           // specify WIFI or ETHERNET depending on if you have Wifi or an EthernetShield; Wifi has to be on Serial1 UDP or TCP for the protocol 
  // <obj>.setHttpCallback(httpRequestHandler);  // The network interface will provide and HTTP request object which can be used as well to send the reply. cf. example above
  // <obj>.setup(WIFI, MQTT, 8888);              // sending over MQTT.
  // <obj>.setup(WIFI, UDP, 8888);               // Setup without port will use the by default port 2560 :: DOES NOT WORK 
  // <obj>.setup(WIFI);                          // setup without port and protocol will use by default TCP on port 2560 
  // <obj>.setup();                              // all defaults ETHERNET, TCP on port 2560

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