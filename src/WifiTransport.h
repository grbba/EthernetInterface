
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

#ifndef WifiTransport_h
#define WifiTransport_h

#include <Arduino.h>
#include <WiFiEspAT.h>

#include "Singelton.h"
#include "Transport.h"


class WifiTransport: public Singleton<WifiTransport>, public Transport
{
    friend WifiTransport* Singleton<WifiTransport>::get();
    friend void Singleton<WifiTransport>::kill();

private:

    WifiTransport (const WifiTransport&){};

    IPAddress dnsip;
    IPAddress ip;
    static WiFiServer server;
    WiFiUDP Udp;
    protocolType p;
    WiFiClient clients[MAX_SOCK_NUM]; 

public:
  
   // void tcpHandler();
   // void udpHandler();

    uint8_t setup();
    void loop();

    WifiTransport(/* args */);
    ~WifiTransport();
};

#endif // !WifiTransport_h