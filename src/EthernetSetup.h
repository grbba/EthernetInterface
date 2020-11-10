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

#ifndef EthernetSetup_h
#define EthernetSetup_h

#include <Ethernet.h>
#include "NetworkSetup.h"

class EthernetSetup: public NetworkSetup {

private:

    EthernetServer*         server = 0;
    EthernetUDP*            udp = 0;

public:

    byte setup();      // sets the TCP server or UDP udp object; returns 1 if the connection was successfull 0 otherwise
    EthernetServer *getTCPServer() {
        return server;
    }
    EthernetUDP *getUDPServer() {
        return udp;
    }

    EthernetSetup();
    EthernetSetup(uint16_t port, protocolType protocol);
    ~EthernetSetup();
};

#endif