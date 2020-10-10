#include <Arduino.h>

#include "DIAG.h"
#include <Ethernet.h>
#include <WiFiEspAT.h>

// #include "DCCEXParser.h"

#include "StringFormatter.h"
#include "Transport.h"

// DCCEXParser ethParser;

static uint8_t buffer[MAX_ETH_BUFFER];
static uint8_t reply[MAX_ETH_BUFFER];

// EthernetClient clients[MAX_SOCK_NUM] = {0};
EthernetClient Transport::eclients[MAX_SOCK_NUM] = {0};


#define MAX_WIFI_SOCK 15
WiFiClient wclients[MAX_WIFI_SOCK];

/**
 * @brief Sending a reply by using the StringFormatter (this will result in every byte send individually which may/will create an important Network overhead).
 * Here we hook back into the DCC code for actually processing the command using a DCCParser. Alternatively we could use MemeStream in order to build the entiere reply
 * before ending it (cf. Scratch pad below)
 * 
 * @param stream    Actually the Client to whom to send the reply. As Clients implement Print this is working
 * @param command   The reply to be send ( echo as in sendReply() )
 * @param blocking  if set to true will instruct the DCC code to not use the async callback functions
 */
void parse(Print *stream, byte *command, bool blocking)
{
    DIAG(F("DCC parsing:            [%e]\n"), command);
    // echo back (as mock parser )
    StringFormatter::send(stream, F("reply to: %s"), command);
}

/**
 * @brief Sending a reply without going through the StringFormatter. Sends the repy in one go
 * 
 * @param client  Client who send the command to which the reply shall be send
 * @param command Command initaliy recieved to be echoed back 
 */
void sendReply(Client *client, char *command)
{
    memset(reply, 0, MAX_ETH_BUFFER); // reset reply
    sprintf((char *)reply, "reply to: %s", command);
    DIAG(F("Response:               [%e]"), (char *)reply);
    if (client->connected())
    {
        client->write(reply, strlen((char *)reply));
        DIAG(F(" send\n"));
    }
};

void Transport::connectionPool(EthernetServer *server)
{
    for (int i = 0; i < Transport::maxConnections; i++)
    {
        eclients[i] = server->accept(); //  EthernetClient(i);
        DIAG(F("\nEthernet connection pool: [%d:%x]"), i, eclients[i]);
    }
}

void Transport::connectionPool(WiFiServer *server)
{
    for (int i = 0; i < Transport::maxConnections; i++)
    {
        wclients[i] = server->accept(); //  EthernetClient(i);
        DIAG(F("\nWifi connection pool:  [%d:%x]"), i, wclients[i]);
    }
}

/*
 * UDP Section : same for Ethenet & Wifi
 */


void Transport::udpHandler()
{
    int packetSize = myudp->parsePacket();
    if (packetSize)
    {
        DIAG(F("\nReceived packet of size:[%d]\n"), packetSize);
        IPAddress remote = myudp->remoteIP();
        DIAG(F("From:                   [%d.%d.%d.%d:"), remote[0], remote[1], remote[2], remote[3]);
        char portBuffer[6];
        DIAG(F("%s]\n"), utoa(myudp->remotePort(), portBuffer, 10)); // DIAG has issues with unsigend int's so go through utoa

        // read the packet into packetBufffer
        myudp->read(buffer, MAX_ETH_BUFFER);
        // terminate buffer properly
        buffer[packetSize] = '\0';

        DIAG(F("Command:                [%s]\n"), buffer);
        // execute the command via the parser
        // check if we have a response if yes then
        // send the reply
        myudp->beginPacket(myudp->remoteIP(), myudp->remotePort());
        parse(myudp, (byte *)buffer, true);
        myudp->endPacket();

        // clear out the PacketBuffer
        memset(buffer, 0, MAX_ETH_BUFFER); // reset PacktBuffer
        return;
    }
}

/*
 * WIFI/TCP Section 
 */

void Transport::tcpHandler(WiFiServer *server)
{
    // loop over the connection pool
    for (int i = 0; i < MAX_WIFI_SOCK; i++)
    {
        if (!wclients[i].connected())
        {
            wclients[i] = server->accept(); // if not connected try again
        }

        if (wclients[i].connected() && wclients[i].available() > 0) // continue only if the client is connected and something is there to read
        {

            int count = wclients[i].read(buffer, MAX_ETH_BUFFER);

            IPAddress remote = wclients[i].remoteIP();
            buffer[count] = '\0'; // terminate the string properly
            DIAG(F("\nReceived packet of size:[%d] from [%d.%d.%d.%d]\n"), count, remote[0], remote[1], remote[2], remote[3]);
            DIAG(F("Client #:               [%d:%x]\n"), i, eclients[i]);
            DIAG(F("Command:                [%s]\n"), buffer);

            // sendReply(&eclients[i], (char *)buffer);
            parse(&wclients[i], (byte *)buffer, true); // test reply using StringFormatter (this will send byte by byte )

            wclients[i].stop();
        }
    }
}

/**
 * @brief As tcpHandler but this time the connections are kept open (thus creating a statefull session) as long as the client doesn't disconnect. A connection
 * pool has been setup beforehand and determines the number of available sessions depending on the network hardware.  Commands crossing packet boundaries will be captured
 * 
 * @param server Pointer to the WiFiServer handling the TCP/IP Stack 
 */
void Transport::tcpSessionHandler(WiFiServer *server)
{
    // get client from the server
    WiFiClient client = server->accept();
    
    // check for new client
    if (client)
    {
        for (byte i = 0; i < Transport::maxConnections; i++)
        {
            if (!wclients[i])
            {
                // On accept() the EthernetServer doesn't track the client anymore
                // so we store it in our client array
                wclients[i] = client;
                DIAG(F("\nNew Client:                [%d:%x]"), i, eclients[i]);
                break;
            }
        }
    }
    // check for incoming data from all possible clients
    for (byte i = 0; i < Transport::maxConnections; i++)
    {
        if (wclients[i] && wclients[i].available() > 0)
        {
            // read bytes from a client
            int count = wclients[i].read(buffer, MAX_ETH_BUFFER-1);
            buffer[count]=0;
            IPAddress remote = client.remoteIP();
            buffer[count] = '\0'; // terminate the string properly
            DIAG(F("\nReceived packet of size:[%d] from [%d.%d.%d.%d]\n"), count, remote[0], remote[1], remote[2], remote[3]);
            DIAG(F("Client #:               [%d]\n"), i);
            DIAG(F("Command:                [%e]\n"), buffer);

           // parse(&(eclients[i]), buffer, true);
            sendReply(&(wclients[i]), (char *)buffer);
        }
        // stop any clients which disconnect
        for (byte i = 0; i < Transport::maxConnections; i++)
        {
            if (wclients[i] && !wclients[i].connected())
            {
                DIAG(F("\nDisconnect client #%d"), i);
                wclients[i].stop();
            }
        }
    }
}

/*
 * Ethernet/TCP Section
 */

/**
 * @brief Handling incomming TCP over the EthernetShield. Everytime we recieved a packet it will be read and replied to for all commands recieved in that packet
 * If there is an incomplete commands across the boundary the command will be dropped as we close the connection here after each req/reply cycle. This allows for reuse of all the
 * available sockets as much as possible and as such would allow for more then 8 clients in // at the cost of msg/sec handled because of the connection/close overhead. A connection
 * pool has been setup beforehand and for each packet a different socketfrom the pool may be used. This is a complete stateless way of interacting
 * 
 * @param EthernetServer* server Pointer to the ethernetServer handling the TCP/IP Stack
 */
void Transport::tcpHandler(EthernetServer *server)
{
    // add the connectionpool setup here with a state if its init or not ?

    // loop over the connection pool. All sockets have been in principle accepted and should be connected
    for (int i = 0; i < Transport::maxConnections; i++)
    {
        if (!eclients[i].connected())
        {
            eclients[i] = server->accept(); // if not connected try again
        }

        if (eclients[i] && eclients[i].available() > 0) // continue only if the client is connected and something is there to read
        {

            int count = eclients[i].read(buffer, MAX_ETH_BUFFER);

            IPAddress remote = eclients[i].remoteIP();
            int remotePort = eclients[i].remotePort();
            char remoteBuffer[7];
            utoa(remotePort, remoteBuffer, 10);
            buffer[count] = '\0'; // terminate the string properly
            DIAG(F("\nReceived packet of size:[%d] from [%d.%d.%d.%d:%s]\n"), count, remote[0], remote[1], remote[2], remote[3], remoteBuffer);
            DIAG(F("Client #:               [%d:%x]\n"), i, eclients[i]);
            DIAG(F("Command:                [%e]\n"), buffer);

            sendReply(&eclients[i], (char *)buffer);
            // parse(&eclients[i], (byte *)buffer, true); // test reply using StringFormatter (this will send byte by byte )

            eclients[i].stop(); // close the connection once the reply has been send
        }
    }
}

/**
 * @brief As tcpHandler but this time the connections are kept open (thus creating a statefull session) as long as the client doesn't disconnect. A connection
 * pool has been setup beforehand and determines the number of available sessions depending on the network hardware.  Commands crossing packet boundaries will be captured
 * 
 * @param EthernetServer* server Pointer to the ethernetServer handling the TCP/IP Stack 
 */
void Transport::tcpSessionHandler(EthernetServer *server)
{
    // get client from the server
    EthernetClient client = server->accept();
    
    // check for new client
    if (client)
    {
        for (byte i = 0; i < Transport::maxConnections; i++)
        {
            if (!eclients[i])
            {
                // On accept() the EthernetServer doesn't track the client anymore
                // so we store it in our client array
                eclients[i] = client;
                DIAG(F("\nNew Client:                [%d:%x]"), i, eclients[i]);
                break;
            }
        }
    }
    // check for incoming data from all possible clients
    for (byte i = 0; i < Transport::maxConnections; i++)
    {
        if (eclients[i] && eclients[i].available() > 0)
        {
            // read bytes from a client
            int count = eclients[i].read(buffer, MAX_ETH_BUFFER-1);
            buffer[count]=0;
            IPAddress remote = client.remoteIP();
            buffer[count] = '\0'; // terminate the string properly
            DIAG(F("\nReceived packet of size:[%d] from [%d.%d.%d.%d]\n"), count, remote[0], remote[1], remote[2], remote[3]);
            DIAG(F("Client #:               [%d]\n"), i);
            DIAG(F("Command:                [%e]\n"), buffer);

           // parse(&(eclients[i]), buffer, true);
            sendReply(&(eclients[i]), (char *)buffer);
            
        }
        // stop any clients which disconnect
        for (byte i = 0; i < Transport::maxConnections; i++)
        {
            if (eclients[i] && !eclients[i].connected())
            {
                DIAG(F("\nDisconnect client #%d"), i);
                eclients[i].stop();
                // eclients[i]=0; // that would kill the connectionPool and thus a freed slot bc the client disconneced can be provided again (?)
            }
        }
    }
}

/*
 * Scratch pad Section
 */

/*
            // Alternative reply mechanism using MemStream thus allowing to send all in one go using the parser
            streamer.setBufferContentPosition(0, 0);

            // Parse via MemBuffer to be replaced by DCCEXparser.parse later

            parse(&streamer, buffer, true); // set to true to that the execution in DCC is sync
            
            if (streamer.available() == 0)
            {
                DIAG(F("No response\n"));
            }
            else
            {
                buffer[streamer.available()] = '\0'; // mark end of buffer, so it can be used as a string later
                DIAG(F("Response:               [%s]\n"), (char *)reply);
                if (clients[i]->connected())
                {
                    clients[i]->write(reply, streamer.available());
                }
            }
*/