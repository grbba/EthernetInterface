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
static Client *clients[MAX_SOCK_NUM] = {0}; // array of pointers to either _eclients or _wclients;

void parse(Print *stream, byte *command, bool blocking)
{
    DIAG(F("DCC parsing:            [%s]\n"), command);
    // echo back (as mock parser )
    StringFormatter::send(stream, F("reply to: %s"), command);
}

void sendReply(Client *client, char *command)
{
    memset(reply, 0, MAX_ETH_BUFFER); // reset reply
    sprintf((char *)reply, "reply to: %s", command);
    DIAG(F("Response:               [%s]\n"), (char *)reply);
    if (client->connected())
    {
        client->write(reply, strlen((char *)reply));
    }
};

// On accept() the EthernetServer doesn't track the client anymore
// so we store it in our client array
bool isNewClient(Client *client)
{
    uint8_t i;

    // check if client exists in the array
    for (i = 0; i < MAX_SOCK_NUM; i++)
    {
        // DIAG(F("Check client [%d:%x:%x]\n"), i, client, clients[i]);
        if (clients[i] == client)
        {
            // found the client in the array
            delay(1);   // strange but without this we loose incomming requests 
            return true;
        }
    }
    // if it doesn't exist take the first free one and store the client in there
    i = 0;
    while (clients[i] && i < MAX_SOCK_NUM)
    {
        i++;
    }
    if (i == MAX_SOCK_NUM)
    {
        DIAG(F("Error: Too many connections\n"));
        return false;
    }
    DIAG(F("New Client [%d:%x]\n"), i, client);
    clients[i] = client;
    return true;
}

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

void tcpHandler2()
{
    // DIAG(F("tcpHandler2 client connected %d\n"), client->connected());
    uint8_t i = 0;

    // check for incoming data from all possible clients and remove the ones disconnected

    for (i = 0; i < MAX_SOCK_NUM; i++)
    {
        // DIAG(F("Reading from [%x]\n"), clients[i]);
        if (clients[i] && clients[i]->available() > 0)
        {
            // read bytes from a client
            int count = clients[i]->read(buffer, MAX_ETH_BUFFER);

            IPAddress remote = clients[i]->remoteIP();
            buffer[count] = '\0'; // terminate the string properly
            DIAG(F("\nReceived packet of size:[%d] from [%d.%d.%d.%d]\n"), count, remote[0], remote[1], remote[2], remote[3]);
            DIAG(F("Client #:               [%d:%d]\n"), i, clients[i]->connected());
            DIAG(F("Command:                [%s]\n"), buffer);

            // sendReply(clients[i], (char *)buffer);            // test reply outside of StringFormatter ( sending reply in one go )
            parse(clients[i], (byte *)buffer, true);             // test reply using StringFormatter (this will send byte by byte )    
            //ethParser.parse(clients[i], (byte *)buffer, true); // Intergation point into CVReader

            /*
            // Alternative sending mechanism using MemStream thus allowing to send all in one go
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
            // parse(clients[i], (byte *)buffer, true);
            clients[i]->flush();
        */
        }

        // stop any clients which disconnect
        for (i = 0; i < MAX_SOCK_NUM; i++)
        {
            if (clients[i] != 0 && !clients[i]->connected())
            {
                DIAG(F("Disconnect client #%d \n"), i);
                clients[i]->stop();
                clients[i] = 0;
            }
        }
    }
}

void Transport::tcpHandler(WiFiServer *server)
{
    // get client from the server
    WiFiClient _wclient = server->accept();
    Client *client = &_wclient;

    if (_wclient) // continue only if the client is connected
    {
        isNewClient(client);    // if its a new one add to the list of clients
        tcpHandler2();          // handle the incomming packages from all connected clients
        _wclient.stop();        // close the connection after having handled the request
    }
}

void Transport::tcpHandler(EthernetServer *server)
{
    // get client from the server
    EthernetClient _eclient = server->accept();
    Client *client = &_eclient;

    if (_eclient) // continue only if the client is connected
    {
        isNewClient(client);  // if it's a new one add to the list of clients
        tcpHandler2();        // handle the incomming packages from all connected clients
        _eclient.stop();      // close the connection after having handled the request

    }
}
