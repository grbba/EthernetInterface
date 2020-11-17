/*
 * © 2020 Gregor Baues. All rights reserved.
 *  
 * This is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 * See the GNU General Public License for more details <https://www.gnu.org/licenses/>
 */

#include <Arduino.h>

#include "NetworkDiag.h"
#include "NetworkInterface.h"
#include "Transport.h"

#ifdef CLI_ENABLED

#include "cli/LoginShell.h"
#include "cli/Shell.h"
#include "cli/CLIcommands.h"

Shell shell;

#endif

#ifdef DCCEX_ENABLED
#include "RingStream.h"
#endif

extern bool diagNetwork;
extern uint8_t diagNetworkClient;


/**
 * @brief Initalizes either the TCP or WiFi transport and creates the connecion pool. If the port for this transport is 23 and the protocol TCP it will start also the
 * telnet server and set the maxConnections to 1 ( only one telnet connection will be allowed ) 
 * 
 * @tparam S Server type
 * @tparam C Client type
 * @tparam U UDP type
 * @param nw NetworkInterface for this transport instance
 * @return true 
 * @return false 
 */

template<class S, class C, class U> 
bool Transport<S,C,U>::setup(NetworkInterface *nw) {
    t = new TransportProcessor();

    if (protocol == TCP) { 
#ifdef CLI_ENABLED
        if (port == 23){ 
            INFO(F("CS-CLI session enabeled ...")); 
            WARN(F("Connections limited to 1 ..."));
            // networkInterface used for Telnet session so limit the connections to 1
            // when we get a new client on port 23; Terminal used shall be set to no echo & CRLF 
            maxConnections = 1;
        }
#endif
        connectionPool(server);     // server should have started here so create the connection pool only for TCP though
        t->udp = nullptr;
    } else {
        connectionPool(udp);
        t->udp = udp;
    }
    
    t->nwi = nw;                    // The TransportProcessor needs to know which Interface he is connected to
    connected = true;               // server & clients which will recieve/send data have all e setup and are available
    return true;
} 

template<class S, class C, class U> 
void Transport<S,C,U>::loop() {
    switch (protocol)
    {
    case UDPR:
    {
        udpHandler(udp);
        break;
    };
    case TCP:
    {
        DBG(F("Transport: %s"), this->transport == WIFI ? "WIFI" : "ETHERNET"); 
        tcpSessionHandler(server);
    };
    case MQTT:
    {
        // MQTT
        break;
    };
    }
}

template<class S, class C, class U> 
void Transport<S, C, U>::connectionPool(S *server)
{
    for (int i = 0; i < Transport::maxConnections; i++)
    {
        clients[i] = server->accept();
        connections[i].client = &clients[i];              
        memset(connections[i].overflow, 0, MAX_OVERFLOW); 
        connections[i].id = i;
        TRC(F("TCP Connection pool:       [%d:%x]"), i, connections[i].client);
    }
#ifdef CLI_ENABLED
    if ( port == 23 && !CLI::connected()) {
        CLI::connect(&connections[0]);
    }
#endif

}
template<class S, class C, class U> 
void Transport<S, C, U>::connectionPool(U *udp)
{
    for (int i = 0; i < Transport::maxConnections; i++)
    {             
        memset(connections[i].overflow, 0, MAX_OVERFLOW); 
        connections[i].id = i;

        TRC(F("UDP Connection pool:       [%d:%x]"), i, udp);
    }
}
/**
 * @todo implement UDP properly
 * 
 * @tparam S Server (either EthernetServer or WiFiServer)
 * @tparam C Client (either EthernetClient or WiFiClient)
 * @tparam U UDP (either EthernetUDP or WiFiUDP)
 */

template<class S, class C, class U> 
void Transport<S, C, U>::udpHandler(U* udp)
{
    int packetSize = udp->parsePacket();
    if (packetSize > 0)
    {
        TRC(F("Received packet of size:[%d]"), packetSize);
        IPAddress remote = udp->remoteIP();
        char portBuffer[6];
        TRC(F("From: [%d.%d.%d.%d: %s]"), remote[0], remote[1], remote[2], remote[3], utoa(udp->remotePort(), portBuffer, 10)); // DIAG has issues with unsigend int's so go through utoa

        udp->read(t->buffer, MAX_ETH_BUFFER);   
        t->buffer[packetSize] = 0;           // terminate buffer
        t->readStream(&connections[0], false );  // there is only one connection for UDP; reading into the buffer has been done

        memset(t->buffer, 0, MAX_ETH_BUFFER);   // reset PacktBuffer
        return; 
    }
    return;
}

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
        for (byte i = 0; i < maxConnections; i++)
        {
            if (!clients[i])
            {
                // On accept() the EthernetServer doesn't track the client anymore
                // so we store it in our client array
                
                clients[i] = client;
                INFO(F("New Client: [%d:%x]"), i, clients[i]);
#ifdef CLI_ENABLED
                if (port == 23 ){
                    // new telnet client so start the shell
                    
                    // shell.setMachineName("cs-excli");
                    shell.begin(clients[0], 5);  // 5 lines of history in the shell
                    shell.println("Welcome to the CommandStation EX Command Line Interface\r\n");
                    shell.flush();
                }
#endif
                break;
            }
        }
    }

    // check for incoming data from all possible clients
    for (byte i = 0; i < maxConnections; i++)
    {
        if (clients[i] && clients[i].available() > 0)
        {
#ifdef CLI_ENABLED
            if (port == 23 ) {
                shell.loop();
            } else {
#endif
            t->readStream(&connections[i], true);
#ifdef CLI_ENABLED
            }
#endif 
        }
        // stop any clients which disconnect
        for (byte i = 0; i < maxConnections; i++)
        {
            if (clients[i] && !clients[i].connected())
            {
                INFO(F("Disconnect client #%d"), i);
                clients[i].stop();
                connections[i].isProtocolDefined = false;
                if (diagNetworkClient == i && diagNetwork) 
                {
                    diagNetwork = false;
                    NetworkDiag::resetDiagOut();
                }
            }
        }
    }
}

template<class S, class C, class U> 
Transport<S,C,U>::Transport(){}
template<class S, class C, class U> 
Transport<S,C,U>::~Transport(){}

// explicitly instatiate to get the relevant copies for ethernet / wifi build @compile time
template class Transport<EthernetServer,EthernetClient,EthernetUDP>;
template class Transport<WiFiServer, WiFiClient, WiFiUDP>;
