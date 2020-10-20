
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

#ifndef Transport_h
#define Transport_h

#include <Arduino.h>
#include <Ethernet.h>
#include <WiFiEspAT.h>

#include <NetworkInterface.h>

#define MAX_ETH_BUFFER 64                   // maximum length we read in one go from a TCP packet. Anything longer in one go send to the Arduino may result in unpredictable behaviour.
                                            // idealy the windowsize should be set accordingly so that the sender knows to produce only max 250 size packets. 
#define MAX_SOCK_NUM 8                      // Maximum number of sockets allowed for any WizNet based EthernetShield. The W5100 only supports 4
#define MAX_WIFI_SOCK 5                     // ESP8266 doesn't support more than 5 connections in //
#define LISTEN_PORT 2560                    // default listen port for the server
#define MAX_OVERFLOW MAX_ETH_BUFFER/2       // length of the overflow buffer to be used for a given connection.
#define MAX_JMRI_CMD 32                     // MAX Length of a JMRI Command 

#define MAC_ADDRESS                        \
    {                                      \
        0x52, 0xB8, 0x8A, 0x8E, 0xCE, 0x21 \
    }                            // MAC address of your networking card found on the sticker on your card or take one from above
#define IP_ADDRESS 10, 0, 0, 101 // Just in case we don't get an adress from DHCP try a static one;

// Emulate Serial1 on pins 6/7 if not present
#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL1)
#include <SoftwareSerial.h>
SoftwareSerial Serial1(6, 7); // RX, TX
#define AT_BAUD_RATE 9600
#else
#define AT_BAUD_RATE 115200
#endif

/**
 * @brief  Application level protocol; detemined upon first connection i.e. first message recieved
 * 
 * @todo   Check if MQTT makes sens here
 * 
 */

typedef enum {
    DCCEX,              // if char[0] = < opening bracket the client should be a JMRI / DCC EX client_h
    HTTP,               // If char[0] = G || P || D; if P then char [1] = U || O || A 
    WITHROTTLE,         // if char[0] = N 
    UNKNOWN_PROTOCOL
} appProtocol;

using appProtocolCallback = void(*)(uint8_t connection);

struct Connection {
    Client *client;
    char overflow[MAX_OVERFLOW];
    appProtocol p;
    bool isProtocolDefined = false;
    appProtocolCallback appProtocolHandler;         
};

/**
 * @brief templated transport class 
 * 
 * @tparam S Server ( EthernetServer or WiFiServer )
 * @tparam C Client ( EthernetClient or WiFiClient )
 * @tparam U UDP class ( EthernetUDP or WiFiUDP )
 */


template <class S, class C, class U> class Transport 
{

private:
    C               clients[MAX_SOCK_NUM];
    Connection      connections[MAX_SOCK_NUM];
    uint8_t         maxConnections;   
    bool            connected;
    U               udp; 
    // S*              server;
    uint8_t         mac[6]; // = MAC_ADDRESS;
    IPAddress       dnsip;
    IPAddress       ip;

    void commandHandler(C* client, uint8_t c, char delimiter);
    void readStream(Connection* c, uint8_t i);
    void udpHandler();
    // void tcpHandler(S* server);             // not used currently   
    void tcpSessionHandler(S* server);      // tcpSessionHandler -> connections are maintained open until close by the client
    void connectionPool(S* server);         // allocates the Sockets at setup time and creates the Connections

    // tag for setup dispatch
    template<class T>
    struct setupTag{};

    void setupHelper(setupTag<EthernetServer>);
    void setupHelper(setupTag<WiFiServer>);

    bool setupEthernet();
    bool setupWifi();

public:
    uint16_t        port;
    uint8_t         protocol;               // TCP or UDP  
    uint8_t         transport;              // WIFI or ETHERNET 
    S*              server;
    // WiFiServer*     wServer = 0;
    // EthernetServer* eServer = 0;                               

    uint8_t setup();
    void loop(); 

    Transport<S,C,U>();
    ~Transport<S,C,U>();
    
};



#endif // !Transport_h