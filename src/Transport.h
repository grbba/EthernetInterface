#ifndef Transport_h
#define Transport_h

#include <Arduino.h>
#include <Ethernet.h>
#include <WiFiEspAT.h>

#include "MemStream.h"

#define MAX_ETH_BUFFER 250
#define MAX_SOCK_NUM 8
#define MAX_WIFI_SOCK 15
#define LISTEN_PORT 2560 // default listen port for the server

class Transport
{
private:
    uint8_t test;
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