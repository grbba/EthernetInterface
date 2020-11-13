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

#ifndef WifiSetup_h
#define WifiSetup_h

#include "NetworkSetup.h"
#include "WiFiEspAT.h"

// Emulate Serial1 on pins 6/7 if not present
#if defined(ARDUINO_ARCH_AVR) && !defined(HAVE_HWSERIAL1)
#include <SoftwareSerial.h>
SoftwareSerial Serial1(6, 7); // RX, TX
#define AT_BAUD_RATE 9600
#else
#define AT_BAUD_RATE 115200
#endif

/**
 * Setting up an TCP/UDP WiFi connection. Stores and provides acces to the Server and UDP instance currently 
 * setup. Once the setup is done for a given NetworkInterface the instance of this class is not needed anymore.
 * An instance will be created within the NetworInterface setup function ans go out of scope after the setup has been done.
 */
class WifiSetup: public NetworkSetup {

private:

    WiFiServer*         server;
    WiFiUDP*            udp;

public:

    bool setup();

    WiFiUDP* getUDPServer() {
        return udp;
    }

    WiFiServer* getTCPServer() {
        return server;
    }

    WifiSetup();
    WifiSetup(uint16_t port, protocolType protocol);
    ~WifiSetup();
};

#endif