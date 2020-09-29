#include "Arduino.h"
#include "DIAG.h"

#include "NetworkInterface.h"
#include "WifiTransport.h"
#include "EthernetTransport.h"

protocolType NetworkInterface::p;
transportType NetworkInterface::t;
uint16_t NetworkInterface::port;

void NetworkInterface::setup(transportType tt, protocolType pt, uint16_t localPort)
{
    uint8_t ok = 0;

    DIAG(F("\nNetwork Setup In Progress ...\n"));
    port = localPort; // set port
    p = pt;           // set protocol
    t = tt;           // set transport

    DIAG(F("\n[%s] Transport Setup In Progress ...\n"), tt ? "Ethernet" : "Wifi");

    switch (tt)
    {
    case WIFI:
    {
        transport = Singleton<WifiTransport>::get();
        ok = 1;
        break;
    };
    case ETHERNET:
    {
        transport = Singleton<EthernetTransport>::get();
        ok = 1;
        break;
    };
    default:
    {
        DIAG(F("\nERROR: Unknown Transport"));// Something went wrong
        break;
    }
    }

    if(ok) {  // continue
        ok = transport->setup(p,port);
    }

    DIAG(F("\n\n[%s] Transport %s ..."), tt ? "Ethernet" : "Wifi", ok ? "Succes" : "Failed");
    DIAG(F("\nNetwork Setup done ..."));
}

void NetworkInterface::loop() {

    transport->loop();

}


NetworkInterface::NetworkInterface()
{
    // DIAG(F("NetworkInterface created "));
}

NetworkInterface::~NetworkInterface()
{
    // DIAG(F("NetworkInterface destroyed"));
}

void NetworkInterface::setTest(uint8_t v)
{
    test = v;
}

uint8_t NetworkInterface::getTest()
{
    return test;
}