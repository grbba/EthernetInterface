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

// #include "MemoryFree.h"

#include "DIAG.h"
#include "freeMemory.h"

#include "NetworkInterface.h"



// (0) Declare NetworkInterfaces
NetworkInterface nwi1;
NetworkInterface nwi2;
// (0) Declared NetworkInterfaces

// (1) Start NetworkInterface - HTTP callback
void httpRequestHandler(ParsedRequest *req, Client* client) {
  DIAG(F("\nParsed Request:"));
  DIAG(F("\nMethod:         [%s]"), req->method);
  DIAG(F("\nURI:            [%s]"), req->uri);
  DIAG(F("\nHTTP version:   [%s]"), req->version);
  DIAG(F("\nParameter count:[%d]\n"), *req->paramCount);
}
// (1) End NetworkInterface - HTTP callback

void setup()
{
  // The main sketch has responsibilities during setup()

  // Responsibility 1: Start the usb connection for diagnostics
  // This is normally Serial but uses SerialUSB on a SAMD processor

  Serial.begin(115200);
  DIAG(F("DCC++ EX NetworkInterface Standalone"));
  

  // (2) Start NetworkInterface - The original WifiInterface is still there but disabled

  DIAG(F("\nFree RAM before network init: [%d]\n"),freeMemory());
  DIAG(F("\nNetwork Setup In Progress ...\n\n"));
  
  // WIFI, TCP on Port 2560, Wifi (ssid/password) has been configured permanetly already on the esp. If
  // the connection fails will go into AP mode 
  // wifi.setup(WIFI);   

  // New connection on known ssid / password combo / port can be added as a last parameter other wise the default of 2560
  // will be used. If it passes the connection will be stored as permanent default. If fails will go into AP mode.                
  // wifi.init(WIFI, TCP, F(WIFI_SSID), F(WIFI_PASSWORD), F(WIFI_HOSTNAME));
  // wifi.init(WIFI, TCP, F(WIFI_SSID), F(WIFI_PASSWORD), F(WIFI_HOSTNAME, 2323)
  // wifi.init


  // nwi1.setup(ETHERNET, UDPR);                    // ETHERNET/UDP on Port 2560 
  // nwi2.setup(ETHERNET, UDPR, 8888);              // ETHERNET/UDP on Port 8888 
  // nwi1.setup(ETHERNET, TCP);                     // ETHERNET/TCP on Port 2560 
  nwi2.setup(ETHERNET, TCP, 23);                  // ETHERNET/TCP on Port 23 for the CLI
  // nwi1.setup(ETHERNET, TCP, 8888);               // ETHERNET/TCP on Port 8888
  // nwi2.setup(WIFI, TCP);                         // WIFI/TCP on Port 2560
  // nwi1.setHttpCallback(httpRequestHandler);      // HTTP callback

  DIAG(F("\nNetwork Setup done ...\n"));
  DIAG(F("\nFree RAM after network init: [%d]\n"),freeMemory());

  // (2) End starting NetworkInterface

}

void loop()
{

// (3) Start Loop NetworkInterface 
NetworkInterface::loop();
// (3) End Loop NetworkInterface
  
// Optionally report any decrease in memory (will automatically trigger on first call)
#if ENABLE_FREE_MEM_WARNING
  static int ramLowWatermark = 32767; // replaced on first loop 

  int freeNow = freeMemory();
  if (freeNow < ramLowWatermark)
  {
    ramLowWatermark = freeNow;
    LCD(2,F("Free RAM=%5db"), ramLowWatermark);
  }
#endif
}
