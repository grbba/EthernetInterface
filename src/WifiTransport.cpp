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
#include <WiFiEspAT.h>

#include "DIAG.h"
#include "StringFormatter.h"

#include "NetworkInterface.h"
#include "WifiTransport.h"

// Emulate Serial1 on pins 6/7 if not present
#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL1)
#include <SoftwareSerial.h>
SoftwareSerial Serial1(6, 7); // RX, TX
#define AT_BAUD_RATE 9600
#else
#define AT_BAUD_RATE 115200
#endif

WiFiServer WifiTransport::server = WiFiServer(LISTEN_PORT);

uint8_t WifiTransport::setup()
{

    p = (protocolType)protocol;

    Serial1.begin(AT_BAUD_RATE);
    WiFi.init(Serial1);

    maxConnections = MAX_WIFI_SOCK;

    if (WiFi.status() == WL_NO_MODULE)
    {
        DIAG(F("Communication with WiFi module failed!\n"));
        return 0;
    }

    DIAG(F("Waiting for connection to WiFi "));
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        DIAG(F("."));
    }

    // Setup the protocol handler
    DIAG(F("\n\nNetwork Protocol:      [%s]"), p ? "UDP" : "TCP");

    switch (p)
    {
    case UDP:
    {
        myudp = &Udp;
        if (Udp.begin(port))
        {
            connected = true;
            ip = WiFi.localIP();
        }
        else
        {
            DIAG(F("\nUDP client failed to start"));
            connected = false;
        }
        break;
    };
    case TCP:
    {
        server = WiFiServer(port);
        server.begin(MAX_WIFI_SOCK, 240);
        if(server.status()) {
            connected = true;
            ip = WiFi.localIP();
        } else {
            DIAG(F("\nWiFi server failed to start"));
            connected = false;
        } // Connection pool not used for WiFi
        break;
    };
    case MQTT: {
        // do the MQTT setup stuff here 
    };
    default:
    {
        DIAG(F("Unkown Ethernet protocol; Setup failed"));
        connected = false;
        break;
    }
    }

    if (connected)
    {
        DIAG(F("\nLocal IP address:      [%d.%d.%d.%d]"), ip[0], ip[1], ip[2], ip[3]);
        DIAG(F("\nListening on port:     [%d]"), port);
        dnsip = WiFi.dnsServer1();
        DIAG(F("\nDNS server IP address: [%d.%d.%d.%d] "), dnsip[0], dnsip[1], dnsip[2], dnsip[3]);
        DIAG(F("\nNumber of connections: [%d]"), maxConnections);
        return 1;
    }
    // something went wrong
    return 0;
}

void WifiTransport::loop()
{

    switch (p)
    {
    case UDP:
    {
        udpHandler();
        break;
    };
    case TCP:
    {
        // tcpHandler(&server);
        tcpSessionHandler(&server);
        break;
    };
    case MQTT:
    {
        break;
    };
    }
}

WifiTransport::WifiTransport()
{
    // DIAG(F("WifiTransport created "));
}

WifiTransport::~WifiTransport()
{
    // DIAG(F("WifiTransport destroyed"));
}