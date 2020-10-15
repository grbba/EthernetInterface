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
#include "WifiTransport.h"
#include "EthernetTransport.h"

Transport* NetworkInterface::transport;
HttpCallback NetworkInterface::httpCallback;

void NetworkInterface::setup(transportType tt, protocolType pt, uint16_t lp)
{
    uint8_t ok = 0;

    DIAG(F("\n[%s] Transport Setup In Progress ...\n"), tt ? "Ethernet" : "Wifi");

    switch (tt)
    {
    case WIFI:
    {
        transport = Singleton<WifiTransport>::get();
        ok = 1;
        break;
    };
    case ETHERNET:
    {
        transport = Singleton<EthernetTransport>::get();
        ok = 1;
        break;
    };
    default:
    {
        DIAG(F("\nERROR: Unknown Transport"));// Something went wrong
        break;
    }
    }

    if(ok) {  // configure the Transport and get it up and running
        transport->port = lp;
        transport->protocol = pt;
        // ok = transport->setup(pt,lp);
        ok = transport->setup();
    }

    DIAG(F("\n\n[%s] Transport %s ..."), tt ? "Ethernet" : "Wifi", ok ? "OK" : "Failed");
    
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

    transport->loop();

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