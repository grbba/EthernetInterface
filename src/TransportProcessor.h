
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

#ifndef TransportProcesor_h
#define TransportProcessor_h

#include <Arduino.h>
// #include <Ethernet.h>
// #include <WiFiEspAT.h>

// #include <NetworkInterface.h>


class TransportProcessor 
{

private:
    static uint8_t buffer[MAX_ETH_BUFFER];
    char command[MAX_JMRI_CMD] = {0};

    void processStream(Connection* c);
    void jmriProcessor();

public:
    
    void readStream(Connection *c);     // reads incomming packets and hands over to the commandHandle for taking the stream apart for commands
    void processStream(Connection* c); 

    TransportProcessor();
    ~TransportProcessor();
    
};

#endif // !Transport_h