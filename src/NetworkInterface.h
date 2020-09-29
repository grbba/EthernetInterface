// Singelton pattern but not thread nor copy safe
// shouldn't be an issue for the thread but copy 
// may have to be looked into
// same pattern shall apply for the Wifi/Ethernet connection


#ifndef NetworkInterface_h
#define NetworkInterface_h

#include "Arduino.h"
#include "Singelton.h"
#include "Transport.h"

typedef enum {
    TCP,
    UDP
} protocolType;

typedef enum  {
    WIFI,                   // using an AT (Version >= V1.7) command enabled ESP8266 not to be used in conjunction with the WifiInterface though! not tested for conflicts
    ETHERNET                   // using the EthernetShield
} transportType;

typedef void (*handlerCallback)();

class NetworkInterface: public Singleton<NetworkInterface>
{
    friend NetworkInterface* Singleton<NetworkInterface>::get();
    friend void Singleton<NetworkInterface>::kill();
private:
    NetworkInterface (const NetworkInterface&){};

    // static  uint16_t port;
    static  protocolType p;
    static  transportType t;
    Transport* transport;

    uint8_t test;

public:
    static  uint16_t port;
    void setTest(uint8_t v);
    uint8_t getTest();

    void setup(transportType t, protocolType p, uint16_t port);        // specific port nummber
    void loop();

    // static void setup(transportType t, protocolType p);                    // uses default port number
    // static void setup(transportType t);                                    // defaults for protocol/port 
    // static void setup();                                                   // defaults for all as above plus CABLE (i.e. using EthernetShield ) as default

    NetworkInterface();
    ~NetworkInterface();
};

#endif