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
#include "TransportProcessor.h"

// DCCEXParser ethParser;

// static uint8_t buffer[MAX_ETH_BUFFER];
// static uint8_t reply[MAX_ETH_BUFFER];


HttpRequest httpReq;
uint16_t _rseq[MAX_SOCK_NUM] = {0};
uint16_t _sseq[MAX_SOCK_NUM] = {0};

char protocolName[4][11] = {"JMRI", "HTTP", "WITHROTTLE", "UNKNOWN"};  // change for Progmem

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
 * @brief Parses the buffer to extract commands to be executed
 * 
 */

void TransportProcessor::processStream(Connection *c)
{
    uint8_t i, j, k, l = 0;

    DIAG(F("\nBuffer: %e"), buffer);
    // copy overflow into the command
    if ((i = strlen(c->overflow)) != 0)
    {
        // DIAG(F("\nCopy overflow to command: %e"), c->overflow);
        strncpy(command, c->overflow, i);
        k = i;
    }
    // reset the overflow
    memset(c->overflow, 0, MAX_OVERFLOW);

    // check if there is again an overflow and copy if needed
    if ((i = strlen((char *)buffer)) == MAX_ETH_BUFFER - 1)
    { // only then we shall be in an overflow situation
        // DIAG(F("\nPossible overflow situation detected: %d "), i);
        j = i;
        while (buffer[i] != c->delimiter)
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
            c->overflow[j] = buffer[i];
            // DIAG(F("\n%d %d %d %c"),k,j,i, buffer[i]); // c->overflow[j]);
        }
        buffer[l] = '\0'; // terminate buffer just after the last '>'
        // DIAG(F("\nNew buffer: [%s] New overflow: [%s]\n"), (char*) buffer, c->overflow );
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
        if (buffer[i] == c->delimiter)
        { // closing bracket need to fix if there is none before an opening bracket ?

            command[k+1] = '\0';

            DIAG(F("Command:                [%d:%d:%e]\n"), c, _rseq[c->id], command);

            // parse(client, buffer, true);
            // sendReply(c->client, command, c);
            memset(command, 0, MAX_JMRI_CMD); // clear out the command

            _rseq[c->id]++;
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
 * @brief Reads what is available on the incomming TCP stream and hands it over to the protocol handler.
 * 
 * @param c    Pointer to the connection struct contining relevant information handling the data from that connection
 */

void TransportProcessor::readStream(Connection *c)
{
    // read bytes from a client
    int count = c->client->read(buffer, MAX_ETH_BUFFER - 1); // count is the amount of data ready for reading, -1 if there is no data, 0 is the connection has been closed
    buffer[count] = 0;

    // figure out which protocol

    if (!c->isProtocolDefined)
    {
        c->p = setAppProtocol(buffer[0], buffer[1]);
        c->isProtocolDefined = true;
        switch (c->p)
        {
        case DCCEX:
        {
            c->delimiter = '>';
            c->appProtocolHandler = (appProtocolCallback)jmriProcessor;
            break;
        }
        case WITHROTTLE:
        {
            c->delimiter = '\n'; 
            c->appProtocolHandler = (appProtocolCallback)withrottleProcessor;
            break;
        }
        case HTTP:
        {
            c->appProtocolHandler = (appProtocolCallback)httpProcessor;
            httpReq.callback = NetworkInterface::getHttpCallback();
            break;
        }
        case UNKNOWN_PROTOCOL:
        {
            DIAG(F("Requests will not be handeled and packet echoed back\n"));
            c->appProtocolHandler = (appProtocolCallback)echoProcessor;
            break;
        }
        }
    }

    IPAddress remote = c->client->remoteIP();

    buffer[count] = '\0'; // terminate the string properly
    DIAG(F("\nReceived packet of size:[%d] from [%d.%d.%d.%d]\n"), count, remote[0], remote[1], remote[2], remote[3]);
    DIAG(F("Client #:               [%d]\n"), c->id);
    DIAG(F("Packet:                 [%e]\n"), buffer);

    // chop the buffer into CS / WiThrottle commands || assemble command across buffer read boundaries
    c->appProtocolHandler(c->id);
}
