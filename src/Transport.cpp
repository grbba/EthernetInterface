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
// #include "DCCEXParser.h"

#include "NetworkInterface.h"
#include "HttpRequest.h"
#include "Transport.h"


// DCCEXParser ethParser;

static uint8_t buffer[MAX_ETH_BUFFER];
static uint8_t reply[MAX_ETH_BUFFER];


HttpRequest httpReq;
uint16_t _rseq[MAX_SOCK_NUM] = {0};
uint16_t _sseq[MAX_SOCK_NUM] = {0};

// char protocolName[4][11] = {"JMRI", "HTTP", "WITHROTTLE", "UNKNOWN"};  // change for Progmem


template<class S, class C, class U> 
bool Transport<S,C,U>::setup() {
    // server should have started here so create the connection pool
    connectionPool(server);         
    t = new TransportProcessor();
    connected = true;           // server & clients which will recieve/send data have all e setup and are available
    return true;
} 

template<class S, class C, class U> 
void Transport<S,C,U>::loop() {
    switch (protocol)
    {
    case UDP:
    {
        udpHandler();
        break;
    };
    case TCP:
    {
        // tcpHandler(&server);         // for stateless coms
        tcpSessionHandler(server);     // for session oriented coms
        break;
    };
    case MQTT:
    {
        // MQTT
        break;
    };
    }
}

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

template<class S, class C, class U> 
void Transport<S, C, U>::connectionPool(S *server)
{
    for (int i = 0; i < Transport::maxConnections; i++)
    {
        clients[i] = server->accept();
        connections[i].client = &clients[i];
        connections[i].id = i;
        DIAG(F("\nConnection pool: [%d:%x]"), i, clients[i]);
    }
}

template<class S, class C, class U> 
void Transport<S, C, U>::udpHandler()
{
    int packetSize = udp.parsePacket();
    if (packetSize)
    {
        DIAG(F("\nReceived packet of size:[%d]\n"), packetSize);
        IPAddress remote = udp.remoteIP();
        DIAG(F("From:                   [%d.%d.%d.%d:"), remote[0], remote[1], remote[2], remote[3]);
        char portBuffer[6];
        DIAG(F("%s]\n"), utoa(udp.remotePort(), portBuffer, 10)); // DIAG has issues with unsigend int's so go through utoa

        // read the packet into packetBufffer
        udp.read(buffer, MAX_ETH_BUFFER);
        // terminate buffer properly
        buffer[packetSize] = '\0';

        DIAG(F("Command:                 [%s]\n"),buffer);
        // execute the command via the parser
        // check if we have a response if yes then
        // send the reply
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        parse(&udp, (byte *)buffer, true);
        udp.endPacket();

        // clear out the PacketBuffer
        memset(buffer, 0, MAX_ETH_BUFFER); // reset PacktBuffer
        return;
    }
}



/**
 * @brief Don't know which protocol we use so just echo back the recieved packet
 * 
 */
void echoHandler(Client *client, uint8_t c)
{
    
}

/**
 * @brief Parses the buffer to extract commands to be executed
 * 
 * @tparam S         Server class
 * @tparam C         Client class
 * @tparam U         UDP class       
 * @param c          Client Id
 * @param delimiter  End delimiter for the commands to extract
 */
template<class S, class C, class U> 
void Transport<S, C, U>::commandHandler(uint8_t c, char delimiter) 
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
            sendReply(connections[c].client, command, c);
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
// template<class S, class C, class U> 
void withrottleHandler(uint8_t c)
{
    commandHandler(c, '\n');
}

/**
 * @brief Breaks up packets into JMRI commands
 * 
 * @param client    Client object from whom we receievd the data
 * @param c         Id of the Client object
 */
template<class S, class C, class U> 
void jmriHandler(uint8_t c)
{
    commandHandler(c, '>');
}

/**
 * @brief creates a HttpRequest object for the user callback. Some conditions apply esp reagrding the length of the items in the Request
 * can be found in @file HttpRequest.h 
 *  
 * @param client Client object from whom we receievd the data
 * @param c id of the Client object
 */
void httpHandler(uint8_t c)
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
        // httpReq.callback(&preq, client);
        httpReq.resetRequest();
    } // else do nothing and continue with the next packet
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

template<class S, class C, class U> 
void Transport<S,C,U>::setAppProtocolHandler(Connection *c, appProtocolCallback cb) {
    c->appProtocolHandler = cb;
}

/*
template<class S, class C, class U> 
void Transport<S,C,U>::tcpHandler(S* server)
{
    // loop over the connection pool
    for (int i = 0; i < MAX_WIFI_SOCK; i++)
    {
        if (!clients[i].connected())
        {
            clients[i] = server->accept(); // if not connected try again
        }

        if (clients[i].connected() && clients[i].available() > 0) // continue only if the client is connected and something is there to read
        {
            readStream(&(clients[i]), i);
            clients[i].stop();
        }
    }
}
*/

/**
 * @brief As tcpHandler but this time the connections are kept open (thus creating a statefull session) as long as the client doesn't disconnect. A connection
 * pool has been setup beforehand and determines the number of available sessions depending on the network hardware.  Commands crossing packet boundaries will be captured
 *  
 */

template<class S, class C, class U> 
void Transport<S,C,U>::tcpSessionHandler(S* server)
{
    // get client from the server
    C client = server->accept();

    // check for new client
    if (client)
    {
        for (byte i = 0; i < Transport<S,C,U>::maxConnections; i++)
        {
            if (!clients[i])
            {
                // On accept() the EthernetServer doesn't track the client anymore
                // so we store it in our client array
                clients[i] = client;
                DIAG(F("\nNew Client:                [%d:%x]"), i, clients[i]);
                break;
            }
        }
    }
    // check for incoming data from all possible clients
    for (byte i = 0; i < Transport<S,C,U>::maxConnections; i++)
    {
        if (clients[i] && clients[i].available() > 0)
        {
            // readStream(i);
            t->readStream(&connections[i]);
        }
        // stop any clients which disconnect
        for (byte i = 0; i < Transport<S,C,U>::maxConnections; i++)
        {
            if (clients[i] && !clients[i].connected())
            {
                DIAG(F("\nDisconnect client #%d"), i);
                clients[i].stop();
                connections[i].isProtocolDefined = false;
            }
        }
    }
}

template<class S, class C, class U> 
Transport<S,C,U>::Transport()
{
    // DIAG(F("Transport created "));
}

template<class S, class C, class U> 
Transport<S,C,U>::~Transport()
{
    // DIAG(F("Transport destroyed"));
}

// explicitly instatiate to get the relevant copies for ethernet / wifi build @compile time
template class Transport<EthernetServer,EthernetClient,EthernetUDP>;
template class Transport<WiFiServer, WiFiClient, WiFiUDP>;

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