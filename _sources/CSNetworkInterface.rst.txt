The CommandStation EX NetworkInterface
======================================

.. epigraph::

   No matter where you go, there you are.

   -- Buckaroo Banzai

Summary
-------

The CommandStation-EX NetworkInterface provides Ethernet and/or WiFi capabilities to CommandStation-EX. It supports TCP as well as UDP for sending data containing CommandStationEX recognized protocols i.e. JMRI or WiThrottle. Those commands can be transfered either as raw text or over HTTP.

For more information about CommandStation-EX can be found on `GitHub <https://github.com/DCC-EX/CommandStation-EX>`_ or on the `DCC-EX <https:////dcc-ex.com>`_  website

Features
--------

Networking
^^^^^^^^^^

* **Network types supported** : Ethernet and WiFi
* **Network protocol support** : TCP and UDP
* **Application Transport protcols** : Raw text and HTTP; HTTPS for the ESP only and MQTT are to be done;
* **Application Payload - CommandStation EX protocols**: JMRI format as well as WitHrottle

WiFi
^^^^
* Automatic detection of the WiFi connection setup

Ethernet
^^^^^^^^

* Automatic detection of the EthernetShield capabilities and number of possible concurrent connections for TCP
* UDP will have only one connection available as its a non-connected stateless protocol and all clients will use the same connection
* Ethernet and WiFi can be mixed if a EtherNet Shield and ESP are both connected to the MCU

The interface also supports multiple connections on different ports for TCP and UDP.
You can also mix Ethernet and WiFi ( my setup has actually both connected currently )
WiThrottle over UDP should also support multiple clients as i know the remote IP and ports to reply to to the contrary of TCP as a connected protocol but that i can't test yet

Diagnostics
^^^^^^^^^^^

Dependencies
^^^^^^^^^^^^

* WiFi

The WiFi connection makes use of the WiFiEspAT library from Juraj Andrássy which can be found here on `GitHub <https://github.com/jandrassy/WiFiEspAT>`_ .  

.. note:: This library requires that the ESP, providing the WiFi hardware, supports the AT command set version 1.7 or 2.1 as of today. All different ESP modules should be supported.

Unfortunately most ESP modules shipped today come with an older AT command set installed by default. Therefore flashing your ESP may be required. Plans exist to build
our own ESP WiFi AT library also supporting older AT command sets.
This library is available from the PIO library manager as well as from the Arduino IDE library manager. Please refer to the platformio.ini file on how to use the library. 
Using VSC together with PIO, instead of looking for the library in PIO library manager, can just add the relevant file in your platformio.ini file and the compiler will 
find and download, if required, the library for you.

Instructions for flashing your ESP can be found `here <FlashESP.html>`_

CommandStation EX Integration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are some code changes in the CommandStation EX code required which may be integrated into master eventually;

* Include files
* RingBuffer

Throttles
^^^^^^^^^

Telnet
^^^^^^

Setup
-----

In NetworkConfig.h : uncomment #define DCC_ENABLED; If that is commented out the NetworkInterface runs standalone
Once NetworkInterface is part of the CommandStation EX master this step has been done already


.. attention:: * Memory Consumption: Every network interface instantiated will consume valuable memory/processing time needed also by other parts of the CommandStation. It is recommended to instantiate at most 2. If you don't have enough memory for Accessories, Sensors, Turnouts etc. please look if one network interface would be sufficient.
.. attention:: * For WiFi you need to reserve Serial 1 for the connection to the ESP due to a limitation from the WiFiATEsp library. Otherwise you could use SoftwareSerial (less recommended/feature has to be added)
.. attention:: * UNO is not supported due to ressource limitation. An Arduino Mega is required.
.. attention:: * Individual commands send shall not be longer than half of the MAX_ETH_BUFFER size in order to make sure that no command which spans two recieved packets is lost. The default settings should be sufficient for most use cases.


Roadmap
-------

* Allow user defined application payload protocols
* Integrate MQTT as an application transport protocol
* Add SoftwareSerial to add the WiFi ESP
* AP Access mode for WiFi
* Reset persistent WiFi credentials

Example .ino file for setting up the NetworkInterface
-----------------------------------------------------

.. code-block:: cpp

  #include <Arduino.h>
  #include "DIAG.h"
  #include "freeMemory.h"

  // (0) Include the header file
  #include "NetworkInterface.h"



  // (1) Declare NetworkInterfaces; Two interfaces have been defined here
  NetworkInterface nwi1;
  NetworkInterface nwi2;
  // (1) Declared NetworkInterfaces
  // (2) Start NetworkInterface - HTTP callback

  void httpRequestHandler(ParsedRequest *req, Client* client) {
    DIAG(F("\nParsed Request:"));
    DIAG(F("\nMethod:         [%s]"), req->method);
    DIAG(F("\nURI:            [%s]"), req->uri);
    DIAG(F("\nHTTP version:   [%s]"), req->version);
    DIAG(F("\nParameter count:[%d]\n"), *req->paramCount);
  }
  // (2) End NetworkInterface - HTTP callback

  void setup()
  {
    // The main sketch has responsibilities during setup()

    // Responsibility 1: Start the usb connection for diagnostics
    // This is normally Serial but uses SerialUSB on a SAMD processor

    Serial.begin(115200);
    DIAG(F("DCC++ EX NetworkInterface Standalone"));

    // (3) Start NetworkInterface - The original WifiInterface is still there but disabled

    DIAG(F("\nFree RAM before network init: [%d]\n"),freeMemory());
    DIAG(F("\nNetwork Setup In Progress ...\n\n"));

    nwi1.setup(ETHERNET, TCP, 8888);               // ETHERNET/TCP on Port 8888
    nwi1.setHttpCallback(httpRequestHandler);      // HTTP callback

    nwi2.setup(WIFI, TCP);                         // WIFI/TCP on Port 2560

    DIAG(F("\nNetwork Setup done ...\n"));
    DIAG(F("\nFree RAM after network init: [%d]\n"),freeMemory());

    // (2) End starting NetworkInterface

  }

  void loop()
  {

  // (3) Start Loop NetworkInterface 
  NetworkInterface::loop();
  // (3) End Loop NetworkInterface
  
  }
