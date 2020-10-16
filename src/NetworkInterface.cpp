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
#include "Singelton.h"
// #include "WifiTransport.h"
// #include "EthernetTransport.h"

Transport<WiFiServer,WiFiClient,WiFiUDP>* wifiTransport;
Transport<EthernetServer,EthernetClient,EthernetUDP>* ethernetTransport;

HttpCallback NetworkInterface::httpCallback;

void NetworkInterface::setup(transportType transport, protocolType protocol, uint16_t port)
{
    uint8_t ok = 0;
    tType = transport;

    DIAG(F("\n[%s] Transport Setup In Progress ...\n"), tType ? "Ethernet" : "Wifi");
    // configure the Transport and get it up and running

    switch (tType)
    {
        case WIFI:
        {
            wifiTransport = Singleton<Transport<WiFiServer,WiFiClient,WiFiUDP>>::get();
            wifiTransport->port = port;
            wifiTransport->protocol = protocol;
            ok = wifiTransport->setup();
            break;
        };
        case ETHERNET:
        {
            ethernetTransport = Singleton<Transport<EthernetServer,EthernetClient,EthernetUDP>>::get();
            ethernetTransport->port = port;
            ethernetTransport->protocol = protocol;
            ok = ethernetTransport->setup();
            break;
        };
        default:
        {
            DIAG(F("\nERROR: Unknown Transport"));// Something went wrong
            break;
        }
    }
    DIAG(F("\n\n[%s] Transport %s ..."), tType ? "Ethernet" : "Wifi", ok ? "OK" : "Failed");
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
    switch(tType){
        case WIFI: { wifiTransport->loop(); break;}
        case ETHERNET: { ethernetTransport->loop(); break;}
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