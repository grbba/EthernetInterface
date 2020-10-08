#include <Arduino.h>
#include <Ethernet.h>
#include <utility/w5100.h>

#include "DIAG.h"
#include "NetworkInterface.h"
#include "EthernetTransport.h"


#define MODE 1    // Mode 1 = stateless i.e. all connections get closed after handling 0 = connection get maintained open 

EthernetServer EthernetTransport::server = EthernetServer(LISTEN_PORT);
uint8_t Transport::maxConnections;

uint8_t EthernetTransport::setup()
{
    p = (protocolType)protocol;

    DIAG(F("\nInitialize Ethernet with DHCP"));
    if (Ethernet.begin(mac) == 0)
    {
        DIAG(F("\nFailed to configure Ethernet using DHCP ... Trying with fixed IP"));
        Ethernet.begin(mac, IPAddress(IP_ADDRESS)); // default ip address

        if (Ethernet.hardwareStatus() == EthernetNoHardware)
        {
            DIAG(F("\nEthernet shield was not found. Sorry, can't run without hardware. :("));
            return false;
        };
        if (Ethernet.linkStatus() == LinkOFF)
        {
            DIAG(F("\nEthernet cable is not connected."));
            return false;
        }
    }
    maxConnections = MAX_SOCK_NUM;
    if (Ethernet.hardwareStatus() == EthernetW5100)
    {
        DIAG(F("\nW5100 Ethernet controller detected."));
        maxConnections = 4;  // Max supported officaly by the W5100 but i have been running over 8 as well. Perf has to be evaluated though comparing 4 vs. 8 connections
    }
    else if (Ethernet.hardwareStatus() == EthernetW5200)
    {
        DIAG(F("\nW5200 Ethernet controller detected."));
    }
    else if (Ethernet.hardwareStatus() == EthernetW5500)
    {
        DIAG(F("W5500 Ethernet controller detected."));
    }

    // set the obtained ip address
    ip = Ethernet.localIP();

    // Setup the protocol handler
    DIAG(F("\nNetwork Protocol:      [%s]"), p ? "UDP" : "TCP");
    switch (p)
    {
    case UDP:
    {
        myudp = &Udp;
        if (Udp.begin(port))
        {
            connected = true;
            ip = Ethernet.localIP();
        }
        else
        {
            DIAG(F("\nUDP client failed to start"));
            connected = false;
        }
        break;
    };
    case TCP:
    {
        server = EthernetServer(port);
        server.begin();
        connected = true;
        connectionPool(&server);
        break;
    };
    case MQTT:
    {
        // do the MQTT setup stuff ...
    };
    default:
    {
        DIAG(F("\nUnkown Ethernet protocol; Setup failed"));
        connected = false;
        break;
    }
    }

    if (connected)
    {
        DIAG(F("\nLocal IP address:      [%d.%d.%d.%d]"), ip[0], ip[1], ip[2], ip[3]);
        DIAG(F("\nListening on port:     [%d]"), port);
        dnsip = Ethernet.dnsServerIP();
        DIAG(F("\nDNS server IP address: [%d.%d.%d.%d] "), dnsip[0], dnsip[1], dnsip[2], dnsip[3]);
        DIAG(F("\nNumber of connections: [%d]"), maxConnections);
        return 1;
    }

    // something went wrong
    return 0;
}

void EthernetTransport::loop()
{
    // DIAG(F("Loop .. "));
    switch (p)
    {
    case UDP:
    {
        udpHandler();
        break;
    };
    case TCP:
    {
        tcpHandler(&server);
        break;
    };
    case MQTT:
    {
        // MQTT
        break;
    };
    }
}

EthernetTransport::EthernetTransport()
{
    // DIAG(F("EthernetTransport created "));
}

EthernetTransport::~EthernetTransport()
{
    // DIAG(F("EthernetTransport destroyed"));
}