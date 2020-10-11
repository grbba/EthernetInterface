#ifndef Transport_h
#define Transport_h

#include <Arduino.h>
#include <Ethernet.h>
#include <WiFiEspAT.h>

#include "MemStream.h"

#define MAX_ETH_BUFFER 128  // maximum length we read in one go from a TCP packet. Anything longer in one go send to the Arduino may result in unpredictable behaviour.
                            // idealy the windowsize should be set accordingly so that the sender knows to produce only max 250 size packets. 
#define MAX_SOCK_NUM 8      // Maximum number of sockets allowed for any WizNet based EthernetShield. The W5100 only supports 4
#define MAX_WIFI_SOCK 5     // ESP8266 doesn't support more than 5 connections in //
#define LISTEN_PORT 2560    // default listen port for the server

class Transport
{
private:
    static EthernetClient eclients[MAX_SOCK_NUM];       // WizNet power Ethernet shields have 4 or 8 sockets depending on the version of the Chip
    static WiFiClient wclients[MAX_WIFI_SOCK];          // ESP should have 15(16?) sockets

public:
    uint16_t port;
    uint8_t protocol;
    static uint8_t maxConnections;
    bool connected;
    UDP* myudp;                                     // UDP is abstract WiFiUDP and EthernetUDP inherit both from UDP
    
    uint8_t virtual setup();
    void virtual loop();

 
    void udpHandler();
    void tcpHandler(WiFiServer* server);            // two call with different Signatures here as the Servers do not inherit from a common class
    void tcpHandler(EthernetServer* server);        // tcpHandler -> connections are closed after each recv
    void tcpSessionHandler(EthernetServer* server); // tcpSessionHandler -> connections are maintained open until close by the client
    void tcpSessionHandler(WiFiServer* server);
    void connectionPool(EthernetServer *server);    // allocates the Sockets at setup time
    void connectionPool(WiFiServer *server);

};

#endif // !Transport_h