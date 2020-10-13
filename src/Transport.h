#ifndef Transport_h
#define Transport_h

#include <Arduino.h>
#include <Ethernet.h>
#include <WiFiEspAT.h>

#include "MemStream.h"

#define MAX_ETH_BUFFER 128                  // maximum length we read in one go from a TCP packet. Anything longer in one go send to the Arduino may result in unpredictable behaviour.
                                            // idealy the windowsize should be set accordingly so that the sender knows to produce only max 250 size packets. 
#define MAX_SOCK_NUM 8                      // Maximum number of sockets allowed for any WizNet based EthernetShield. The W5100 only supports 4
#define MAX_WIFI_SOCK 5                     // ESP8266 doesn't support more than 5 connections in //
#define LISTEN_PORT 2560                    // default listen port for the server
#define MAX_OVERFLOW MAX_ETH_BUFFER/2       // length of the overflow buffer to be used for a given connection.
#define MAX_JMRI_CMD 32                     // MAX Length of a JMRI Command 


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

typedef void (*appProtocolCallback)(Client* client, uint8_t connection);

struct Connection {
    Client *client;
    char overflow[MAX_OVERFLOW];
    appProtocol p;
    bool isProtocolDefined = false;
    appProtocolCallback appProtocolHandler;         
};


class Transport
{
private:
    static EthernetClient eclients[MAX_SOCK_NUM];       // WizNet power Ethernet shields have 4 or 8 sockets depending on the version of the Chip KbR02583jEoYOcrJ
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