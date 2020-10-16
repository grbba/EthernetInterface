/*
 *  © 2020, Gregor Baues. All rights reserved.
 *  
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Arduino.h>

#include "DIAG.h"
#include <Ethernet.h>
#include <WiFiEspAT.h>

// #include "DCCEXParser.h"
#include "NetworkInterface.h"
#include "HttpRequest.h"
#include "StringFormatter.h"
#include "Transport.h"

// DCCEXParser ethParser;

static uint8_t buffer[MAX_ETH_BUFFER];
static uint8_t reply[MAX_ETH_BUFFER];

EthernetClient Transport::eclients[MAX_SOCK_NUM] = {0};
WiFiClient Transport::wclients[MAX_WIFI_SOCK];
HttpRequest httpReq;
uint16_t _rseq[MAX_SOCK_NUM] = {0};
uint16_t _sseq[MAX_SOCK_NUM] = {0};

char protocolName[4][11] = {"JMRI", "HTTP", "WITHROTTLE", "UNKNOWN"};
Connection connections[MAX_SOCK_NUM];

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
void sendReply(Client *client, char *command, uint8_t c)
{   
    char *number;
    char seqNumber[6];
    int i = 0;
    
    memset(reply, 0, MAX_ETH_BUFFER);       // reset reply

    number = strrchr(command, ':');         // replace the int after the last ':'
    while( &command[i] != number ) {        // copy command into the reply upto the last ':'
        reply[i] = command[i];
        i++;
    }
    
    strcat((char *)reply, ":");
    itoa(_sseq[c], seqNumber, 10);    
    strcat((char *)reply, seqNumber);
    strcat((char *)reply, ">");

    // sprintf((char *)reply, "reply to: %s", command);

    DIAG(F("Response:               [%e]"), (char *)reply);
    if (client->connected())
    {
        client->write(reply, strlen((char *)reply));
        _sseq[c]++;
        DIAG(F(" send\n"));
    }
};

void Transport::connectionPool(EthernetServer *server)
{
    for (int i = 0; i < Transport::maxConnections; i++)
    {
        eclients[i] = server->accept(); //  EthernetClient(i);
        connections[i].client = &eclients[i];
        DIAG(F("\nEthernet connection pool: [%d:%x]"), i, eclients[i]);
    }
}

void Transport::connectionPool(WiFiServer *server)
{
    for (int i = 0; i < Transport::maxConnections; i++)
    {
        wclients[i] = server->accept(); //  EthernetClient(i);
        connections[i].client = &wclients[i];
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

        DIAG(F("Command:                 [%s]\n"),buffer);
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

/**
 * @brief Set the App Protocol. The detection id done upon the very first message recieved. The client will then be bound to that protocol. Its very brittle 
 * as e.g. The N message as first message for WiThrottle is not a requirement by the protocol; If any client talking Withrottle doesn't implement this the detection 
 * will default to JMRI. For HTTP we base this only on a subset of th HTTP verbs which can be used.
 * 
 * @param a First character of the recieved buffer upon first connection
 * @param b Second character of the recieved buffer upon first connection
 * @return appProtocol 
 */
appProtocol setAppProtocol(char a, char b)
{
    appProtocol p;
    switch (a)
    {
    case 'G': // GET
    case 'C': // CONNECT
    case 'O': // OPTIONS
    case 'T': // TRACE
    {
        p = HTTP;
        break;
    }
    case 'D': // DELETE or D plux hex value
    {
        if (b == 'E')
        {
            p = HTTP;
        }
        else
        {
            p = WITHROTTLE;
        }
        break;
    }
    case 'P':
    {
        if (b == 'T' || b == 'R')
        {
            p = WITHROTTLE;
        }
        else
        {
            p = HTTP; // PUT / PATCH / POST
        }
        break;
    }
    case 'H':
    {
        if (b == 'U')
        {
            p = WITHROTTLE;
        }
        else
        {
            p = HTTP; // HEAD
        }
        break;
    }
    case 'M':
    case '*':
    case 'R':
    case 'Q': // That doesn't make sense as it's the Q or close on app level
    case 'N':
    {
        p = WITHROTTLE;
        break;
    }
    case '<':
    {
        p = DCCEX;
        break;
    }
    default:
    {
        // here we don't know
        p = UNKNOWN_PROTOCOL;
        break;
    }
    }
    DIAG(F("\nClient speaks:          [%s]\n"), protocolName[p]);
    return p;
}

/**
 * @brief Don't know which protocol we use so just echo back the recieved packet
 * 
 */
void echoHandler(Client *client, uint8_t c)
{
}

/**
 * @brief           Breaks up packets into commands according to the delimiter provided. Handles commands possibly
 *                  be distributed over two or (more?) packets. Used for WiThrottle & JMRi
 * 
 * @param client    Client object from whom we receieved the data
 * @param c         Id of the Client object
 * @param delimiter Character used for breaking up a buffer into commands
 */
void commandHandler(Client *client, uint8_t c, char delimiter)
{
    uint8_t i, j, k, l = 0;
    char command[MAX_JMRI_CMD] = {0};
    DIAG(F("\nBuffer: %e"), buffer);
    // copy overflow into the command
    if ((i = strlen(connections[c].overflow)) != 0)
    {
        // DIAG(F("\nCopy overflow to command: %e"), connections[c].overflow);
        strncpy(command, connections[c].overflow, i);
        k = i;
    }
    // reset the overflow
    memset(connections[c].overflow, 0, MAX_OVERFLOW);

    // check if there is again an overflow and copy if needed
    if ((i = strlen((char *)buffer)) == MAX_ETH_BUFFER - 1)
    { // only then we shall be in an overflow situation
        // DIAG(F("\nPossible overflow situation detected: %d "), i);
        j = i;
        while (buffer[i] != delimiter)
        { // what if there is none: ?
            //  DIAG(F("%c"),(char) buffer[i]);
            // Serial.print((char) buffer[i]);
            i--;
        }
        Serial.println();
        i++; // start of the buffer to copy
        l = i;
        k = j - i; // length to copy

        for (j = 0; j < k; j++, i++)
        {
            connections[c].overflow[j] = buffer[i];
            // DIAG(F("\n%d %d %d %c"),k,j,i, buffer[i]); // connections[c].overflow[j]);
        }
        buffer[l] = '\0'; // terminate buffer just after the last '>'
        // DIAG(F("\nNew buffer: [%s] New overflow: [%s]\n"), (char*) buffer, connections[c].overflow );
    }

    // breakup the buffer using its changed length
    i = 0;
    k = strlen(command);            // current length of the command buffer telling us where to start copy in
    l = strlen((char *)buffer);
    // DIAG(F("\nCommand buffer: [%s]:[%d:%d:%d]\n"), command, i, l, k );
    while (i < l)
    {
        // DIAG(F("\nl: %d k: %d , i: %d"), l, k, i);
        command[k] = buffer[i];
        if (buffer[i] == delimiter)
        { // closing bracket need to fix if there is none before an opening bracket ?

            command[k+1] = '\0';

            DIAG(F("Command:                [%d:%d:%e]\n"), c, _rseq[c], command);

            // parse(client, buffer, true);
            sendReply(client, command, c);
            memset(command, 0, MAX_JMRI_CMD); // clear out the command

            _rseq[c]++;
            j = 0;
            k = 0;
        }
        else
        {
            k++;
        }
        i++;
    }
}

/**
 * @brief Breaks up packets into WiThrottle commands
 * 
 * @param client    Client object from whom we receievd the data
 * @param c         Id of the Client object
 */
void withrottleHandler(Client *client, uint8_t c)
{
    commandHandler(client, c, '\n');
}

/**
 * @brief Breaks up packets into JMRI commands
 * 
 * @param client    Client object from whom we receievd the data
 * @param c         Id of the Client object
 */
void jmriHandler(Client *client, uint8_t c)
{
    Serial.println("jmriHandler\n");
    commandHandler(client, c, '>');
}

/**
 * @brief creates a HttpRequest object for the user callback. Some conditions apply esp reagrding the length of the items in the Request
 * can be found in @file HttpRequest.h 
 *  
 * @param client Client object from whom we receievd the data
 * @param c id of the Client object
 */
void httpHandler(Client *client, uint8_t c)
{
    uint8_t i, l = 0;
    ParsedRequest preq;
    l = strlen((char *)buffer);
    for (i = 0; i < l; i++)
    {
        httpReq.parseRequest((char)buffer[i]);
    }
    if (httpReq.endOfRequest())
    {
        preq = httpReq.getParsedRequest();
        httpReq.callback(&preq, client);
        httpReq.resetRequest();
    } // esle do nothing and continue with the next packet
}

/*  This should work but creates a segmentation fault ??

        // check if we have one parameter with name 'jmri' then send the payload directly and don't call the callback
        preq = httpReq.getParsedRequest();
        DIAG(F("Check parameter count\n"));
        if (*preq.paramCount == 1)
        {
            Params *p;
            int cmp;
            p = httpReq.getParam(1);

            DIAG(F("Parameter name[%s]\n"), p->name);
            DIAG(F("Parameter value[%s]\n"), p->value);
            
            cmp = strcmp("jmri", p->name);
            if ( cmp == 0 ) { 
                memset(buffer, 0, MAX_ETH_BUFFER); // reset PacktBuffer
                strncpy((char *)buffer, p->value, strlen(p->value));
                jmriHandler(client, c);
            } else {
                DIAG(F("Callback 1\n"));
                httpReq.callback(&preq, client);
            }
        }
        else
        {
            DIAG(F("Callback 2\n"));
            httpReq.callback(&preq, client);
        }
        DIAG(F("ResetRequest\n"));
        httpReq.resetRequest();

    } // else do nothing and wait for the next packet
}
*/

/**
 * @brief Reads what is available on the incomming TCP stream and hands it over to the protocol handler.
 * 
 * @param client    Client object from whom we receievd the data
 * @param c         Id of the Client object
 */
void readStream(Client *client, byte i)
{
    // read bytes from a client
    int count = client->read(buffer, MAX_ETH_BUFFER - 1); // count is the amount of data ready for reading, -1 if there is no data, 0 is the connection has been closed
    buffer[count] = 0;

    // figure out which protocol

    if (!connections[i].isProtocolDefined)
    {
        connections[i].p = setAppProtocol(buffer[0], buffer[1]);
        connections[i].isProtocolDefined = true;
        switch (connections[i].p)
        {
        case DCCEX:
        {
            connections[i].appProtocolHandler = (appProtocolCallback)jmriHandler;
            break;
        }
        case WITHROTTLE:
        {
            connections[i].appProtocolHandler = (appProtocolCallback)withrottleHandler;
            break;
        }
        case HTTP:
        {
            connections[i].appProtocolHandler = (appProtocolCallback)httpHandler;
            httpReq.callback = NetworkInterface::getHttpCallback();
            break;
        }
        case UNKNOWN_PROTOCOL:
        {
            DIAG(F("Requests will not be handeled and packet echoed back\n"));
            connections[i].appProtocolHandler = (appProtocolCallback)echoHandler;
            break;
        }
        }
    }

    IPAddress remote = client->remoteIP();
    buffer[count] = '\0'; // terminate the string properly
    DIAG(F("\nReceived packet of size:[%d] from [%d.%d.%d.%d]\n"), count, remote[0], remote[1], remote[2], remote[3]);
    DIAG(F("Client #:               [%d]\n"), i);
    DIAG(F("Packet:                 [%e]\n"), buffer);

    // chop the buffer into CS / WiThrottle commands || assemble command across buffer read boundaries
    connections[i].appProtocolHandler(client, i);
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
            readStream(&(wclients[i]), i);
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
                DIAG(F("\nNew Client:             [%d:%x]"), i, eclients[i]);
                break;
            }
        }
    }
    // check for incoming data from all possible clients
    for (byte i = 0; i < Transport::maxConnections; i++)
    {
        if (wclients[i] && wclients[i].available() > 0)
        {
            readStream(&(wclients[i]), i);
        }
        // stop any clients which disconnect
        for (byte i = 0; i < Transport::maxConnections; i++)
        {
            if (wclients[i] && !wclients[i].connected())
            {
                DIAG(F("\nDisconnect client #%d"), i);
                wclients[i].stop();
                connections[i].isProtocolDefined = false;
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
            readStream(&(eclients[i]), i);
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
            readStream(&(eclients[i]), i);
        }
        // stop any clients which disconnect
        for (byte i = 0; i < Transport::maxConnections; i++)
        {
            if (eclients[i] && !eclients[i].connected())
            {
                DIAG(F("\nDisconnect client #%d"), i);
                eclients[i].stop();
                connections[i].isProtocolDefined = false;
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