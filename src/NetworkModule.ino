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
  
  nwi1.setup(WIFI, TCP, 23);               // ETHERNET/TCP on Port 8888
  nwi2.setup(ETHERNET, TCP);                // WIFI/TCP on Port 2560
  
  nwi1.setHttpCallback(httpRequestHandler);      // HTTP callback

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
