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

#ifndef NetworkInterface_h
#define NetworkInterface_h

#include <Arduino.h>

#include "NetworkConfig.h"
#include "HttpRequest.h"

typedef enum protocolType {
    TCP,             //!< TCP standard protocol for the network connection management
    UDPR,            //!< UDP clashes with a class name in the network stack need to define a namespace for that?
    MQTT             //!< MQTT should not be here as its on the application level        
} protocolType;

typedef enum transportType {
    WIFI,                   // using an AT (Version >= V1.7) command enabled ESP8266 not to be used in conjunction with the WifiInterface though! not tested for conflicts
    ETHERNET                // using the EthernetShield
} transportType;

using HttpCallback = void(*)(ParsedRequest *req, Client *client);

/**
 * Abstract parent class of the templated ( Ethernet or Wifi ) Transport class. 
 * Instances of Transport are hold through this in an array in DCCNetwork which describes and 
 * actually manages the available transports.
 */
struct AbstractTransport {
    virtual ~AbstractTransport(){};  //!< declared virtual to make the class abstract
};

/**
 * Core class holding and running the instantiated Transport objects 
 * initalized through the NetworkInterface. The Transports are handled as abstract 
 * transports as they are derived from a templated class. Transport inherits from
 * AbstractTransport. The transports arrayactually holds the instatiated Transport instances.
 * The number of transports is limited by MAX_INTERFACES
 */
class DCCNetwork {
    private:
        byte _tCounter = 0;
        transportType _t[MAX_INTERFACES];
    
    public: 
        AbstractTransport *transports[MAX_INTERFACES];

        byte add(AbstractTransport* t, transportType _t);
        void loop();
};

/**
 * Main entry point used to instatiate the network interface(s). The member functions of this class shall be the only
 * ones used in the main ComandStation-EX sketch. It provides the setup and callback setting facilities as well as the loop
 * managing data transfer and command handling accroding to the CommandStation command protocols available.
 */
class NetworkInterface
{
private:
    HttpCallback httpCallback;
    transportType t;

public:

    void setHttpCallback(HttpCallback callback);                       //!< used for setting the user provided callback function for HTTP request handling
    HttpCallback getHttpCallback();                                   
    void setup(transportType t, protocolType p, uint16_t port);        //!< Used for specific port nummber; default is 2560
    void setup(transportType t, protocolType p);                       //!< uses default port number
    void setup(transportType t);                                       //!< defaults for protocol/port 
    
    void setup();                                                      //!< defaults for all as above plus CABLE (i.e. using EthernetShield ) as default
    static void loop();

    NetworkInterface();
    ~NetworkInterface();
};

#endif