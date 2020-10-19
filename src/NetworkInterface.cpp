/*
 *  © 2020, Gregor Baues. All rights reserved.
 *  
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Arduino.h>

#include "DIAG.h"
#include "NetworkInterface.h"
// #include "Singelton.h"
// #include "WifiTransport.h"
// #include "EthernetTransport.h"

HttpCallback NetworkInterface::httpCallback;

//Transport<WiFiServer,WiFiClient,WiFiUDP> wifiT;
// Transport<EthernetServer,EthernetClient,EthernetUDP> ethernetT;

Transport<WiFiServer, WiFiClient,WiFiUDP>* NetworkInterface::wifiTransport;
Transport<EthernetServer, EthernetClient,EthernetUDP>* NetworkInterface::ethernetTransport;

transportType t;

void NetworkInterface::setup(transportType transport, protocolType protocol, uint16_t port)
{
    
    uint8_t ok = 0;

    DIAG(F("\n[%s] Transport Setup In Progress ...\n"), transport ? "Ethernet" : "Wifi");

    // configure the Transport and get it up and running
    
    t = transport;
    switch (transport)
    {
        case WIFI:
        {
            wifiTransport = new Transport<WiFiServer,WiFiClient,WiFiUDP>;
            wifiTransport->wServer = new WiFiServer(port);
            wifiTransport->port = port;
            wifiTransport->protocol = protocol;
            wifiTransport->transport = transport;
            ok = wifiTransport->setup(); // do the setup here for the server part and remove the setup part from the transport; just set the server accordingly ... 
            break;
        };
        case ETHERNET:
        {
            ethernetTransport = new Transport<EthernetServer,EthernetClient,EthernetUDP>;
            ethernetTransport->eServer = new EthernetServer(port);
            ethernetTransport->port = port;
            ethernetTransport->protocol = protocol;
            ethernetTransport->transport = transport;
            ok = ethernetTransport->setup();
            break;
        };
        default:
        {
            DIAG(F("\nERROR: Unknown Transport"));// Something went wrong
            break;
        }
    }
    DIAG(F("\n\n[%s] Transport %s ..."), transport ? "Ethernet" : "Wifi", ok ? "OK" : "Failed");
}

void NetworkInterface::setup(transportType tt, protocolType pt)
{
    NetworkInterface::setup(tt, pt, LISTEN_PORT);
}

void NetworkInterface::setup(transportType tt)
{
    NetworkInterface::setup(tt, TCP, LISTEN_PORT);
}

void NetworkInterface::setup()
{
    NetworkInterface::setup(ETHERNET, TCP, LISTEN_PORT);
}

void NetworkInterface::loop() {
    switch(t){
        case WIFI: {
            // wifiTransport->loop(); 
            break;
        }
        case ETHERNET: {
            // ethernetTransport->loop();  
            break;
        }
    }
}

void NetworkInterface::setHttpCallback(HttpCallback callback) {
    httpCallback = callback;
}
HttpCallback NetworkInterface::getHttpCallback() {
    return httpCallback; 
}

NetworkInterface::NetworkInterface()
{
    // DIAG(F("NetworkInterface created "));
}

NetworkInterface::~NetworkInterface()
{
    // DIAG(F("NetworkInterface destroyed"));
}