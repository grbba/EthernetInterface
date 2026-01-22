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
#include "NetworkSetup.h"
#include "WifiSetup.h"

void reverseArray(uint8_t arr[], int start, int end);

#if WIFI_AT_ENABLED
/**
 * @brief WiFi specifi setup function. Currently only for Serial1 to connect to the ESP chip.
 * 
 * @return true     if the connection has been established
 * @return false    something went wrong
 */
/**
 * @todo AP setup if no persistent network & credentials found / reset persistent setup & change Network;
 * Forcing AP only mode;
 */

bool WifiSetup::setup() {
    /**
     * @todo setup using SoftwareSerial or any other Hardware Serial port on the mega (i.e. 2 or 3);
     *  Serial3 for the Mega+WiFi combo no jumperds
     *  serial6 on the nucleo with the  makerfabs jumper  
     *  ESP_TX → D0 (so ESP transmits into STM32 RX)
     *  ESP_RX → D1 (so ESP receives from STM32 TX)
     */


// #include <Arduino.h>
// #include <HardwareSerial.h>

#if defined(ARDUINO_ARCH_STM32)
    // Route the configured USART to the desired pins (defaults in NetworkConfig.h).
    HardwareSerial ESP(WIFI_AT_USART);
    ESP.setRx(WIFI_AT_RX_PIN);
    ESP.setTx(WIFI_AT_TX_PIN);
    ESP.begin(AT_BAUD_RATE);
    WiFi.init(ESP);
#else
    Serial1.begin(AT_BAUD_RATE);
    WiFi.init(Serial1);
#endif

    maxConnections = MAX_WIFI_SOCK;

    if (WiFi.status() == WL_NO_MODULE)
    {
        ERR(F("Communication with WiFi module failed!"));
        return false;
    }

    INFO(F("Waiting for connection to WiFi "));
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        DBG(F("."));
    }
    
    INFO(F("Network Protocol: [%s]"), protocol ? "UDP" : "TCP");
    INFO(F("Initialize MAC Addresses ... "));
    WiFi.apMacAddress(apWifiMacAddress);
    reverseArray(apWifiMacAddress, 0, 5);   // the MAc is provided in reverse order ...
    WiFi.macAddress(stWifiMacAddress);
    reverseArray(stWifiMacAddress, 0, 5);

    // Start the TCP/UDP server instance
    switch (protocol)
    {
    case UDPR:
    {
        connected = false;
        /**
         * @todo Can we have multiple UDP instances? Do we always get back the same ? if not possible 
         * we should catch this condition.
         */
        udp = new WiFiUDP(); 
        byte udpState = udp->begin(port);
        if (udpState) 
        {
            TRC(F("UDP status: %d"), udpState);
            maxConnections = 1;            
            connected = true;
        }
        else
        {
            ERR(F("UDP failed to start"));
            connected = false;
        }
        break;
    };
    case TCP:
    {
        server = new WiFiServer(port);
        server->begin(MAX_WIFI_SOCK, 240);
        if(server->status()) {
            connected = true;
        
        } else {
            ERR(F("\nWiFi server failed to start"));
            connected = false;
        } // Connection pool not used for WiFi
        break;
    };
    case MQTT: {
        // do the MQTT setup stuff here 
    };
    default:
    {
        ERR(F("Unkown Ethernet protocol; Setup failed"));
        connected = false;
        break;
    }
    }

    if (connected)
    {
        ip = WiFi.localIP();
        NetworkSetup::printMacAddress(apWifiMacAddress);
        NetworkSetup::printMacAddress(stWifiMacAddress);

        INFO(F("Local IP address:      [%d.%d.%d.%d]"), ip[0], ip[1], ip[2], ip[3]);
        INFO(F("Listening on port:     [%d]"), port);
        dnsip = WiFi.dnsIP();  //   .dnsServer1();
        INFO(F("DNS server IP address: [%d.%d.%d.%d] "), dnsip[0], dnsip[1], dnsip[2], dnsip[3]);
        INFO(F("Number of connections: [%d]"), maxConnections);
        return true;
    }
    return false; // something went wrong

};

void reverseArray(uint8_t arr[], int start, int end)
{
    while (start < end)
    {
        uint8_t temp = arr[start]; 
        arr[start] = arr[end];
        arr[end] = temp;
        start++;
        end--;
    } 
} 

WifiSetup::WifiSetup() {}
WifiSetup::WifiSetup(uint16_t p, protocolType pt ) { port = p; protocol = pt; }
WifiSetup::~WifiSetup() {}
#else
bool WifiSetup::setup() {
    ERR(F("WiFi AT support disabled for this build."));
    return false;
}

void reverseArray(uint8_t arr[], int start, int end)
{
    (void)arr;
    (void)start;
    (void)end;
}

WifiSetup::WifiSetup() {}
WifiSetup::WifiSetup(uint16_t p, protocolType pt ) { port = p; protocol = pt; }
WifiSetup::~WifiSetup() {}
#endif
