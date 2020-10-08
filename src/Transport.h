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
    // void parse(Print *stream, byte *command, bool blocking);

    // static uint8_t buffer[MAX_ETH_BUFFER];
    // static Client* clients[MAX_SOCK_NUM];       // Client is abstract WiFiClient and EthernetClient inherit both from Client

public:
    uint16_t port;
    uint8_t protocol;
    static uint8_t maxConnections;
    bool connected;
    UDP* myudp;                          // UDP is abstract WiFiUDP and EthernetUDP inherit both from UDP
    
    uint8_t virtual setup();
    void virtual loop();

 
    void udpHandler();
    void tcpHandler(WiFiServer* server);        // two call with different Signatures here as the Servers do not inherit from a common class
    void tcpHandler(EthernetServer* server); 
    void connectionPool(EthernetServer *server);
    void connectionPool(WiFiServer *server);

};


#endif // !Transport_h