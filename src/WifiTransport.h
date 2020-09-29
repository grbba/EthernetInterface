#ifndef WifiTransport_h
#define WifiTransport_h

#include "Arduino.h"
#include "Singelton.h"

#include "Transport.h"
#include <WiFiEspAT.h>

class WifiTransport: public Singleton<WifiTransport>, public Transport
{
    friend WifiTransport* Singleton<WifiTransport>::get();
    friend void Singleton<WifiTransport>::kill();

private:
    IPAddress dnsip;
    IPAddress ip;
    WiFiServer *srv;
    WiFiUDP Udp;
    protocolType p;
    WiFiClient clients[MAX_SOCK_NUM]; 

public:
  
   void tcpHandler();
   void udpHandler();

   uint8_t setup(int p, uint16_t port);
   void loop();

    WifiTransport(/* args */);
    ~WifiTransport();
};

#endif // !WifiTransport_h