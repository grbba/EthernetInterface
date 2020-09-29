#ifndef Transport_h
#define Transport_h

#include "Arduino.h"
#include "NetworkInterface.h"
#include "MemStream.h"

typedef void (*handlerCallback)();
#define MAX_ETH_BUFFER 250
#define MAX_SOCK_NUM 8
#define LISTEN_PORT 2560                                        // default listen port for the server 

class Transport
{

public:
    uint16_t port;
    uint8_t buffer[MAX_ETH_BUFFER];
    char packetBuffer[MAX_ETH_BUFFER]; 
    MemStream * streamer;
    bool connected;

    uint8_t virtual setup(int p, uint16_t port);
    void virtual loop();      

};

#endif // !Transport_h