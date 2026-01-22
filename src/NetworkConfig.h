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


// Build configuration

// #define DCCEX_ENABLED            // uncomment to enable CS-EX integration; Commented will operate as standalone and only echo commands as replies
// #define CLI_ENABLED                // enables the Command Line Interface either on serial or any network client; active for network clients if an interface on port 23 eists

// Network operational configuration

#define LISTEN_PORT     2560                                    //!< default listen port for the server
#define MAC_ADDRESS     {0x52, 0xB8, 0x8A, 0x8E, 0xCE, 0x21}    //!< Dummy will not be used; This will be setup automatically based on the unique id of the Arduino
#define IP_ADDRESS      10, 0, 0, 101                           //!< Just in case we don't get an adress from DHCP try a static one; To be adapted to your specific home network setup


// @brief NetworkInterface configuration

#define MAX_INTERFACES  4                                       //!< Consumes too much memory beyond in general not more than 2 should be required 
#define MAX_SOCK_NUM    8                                       //!< Maximum number of sockets allowed for any WizNet based EthernetShield. The W5100 only supports 4 
#define MAX_WIFI_SOCK   5                                       //!< ESP8266 doesn't support more than 5 connections 
#define MAX_ETH_BUFFER  128                                     //!< Maximum length read in one go from a TCP packet. 
#define MAX_OVERFLOW    MAX_ETH_BUFFER / 2                      //!< Length of the overflow buffer to be used for a given connection to make sure we capture commands broken between two recieves 
#define MAX_JMRI_CMD    MAX_ETH_BUFFER / 2                      //!< Maximum Length of a JMRI Command 

#ifndef STRINGFORMATTER_FLOATS
#define STRINGFORMATTER_FLOATS 0
#endif

#ifndef WIFI_AT_ENABLED
#if defined(ARDUINO_ARCH_STM32)
#define WIFI_AT_ENABLED 0
#else
#define WIFI_AT_ENABLED 1
#endif
#endif

// WiFi AT serial config (STM32 defaults; override per board as needed).
#ifndef WIFI_AT_USART
#define WIFI_AT_USART USART6
#endif
#ifndef WIFI_AT_RX_PIN
#define WIFI_AT_RX_PIN PG9
#endif
#ifndef WIFI_AT_TX_PIN
#define WIFI_AT_TX_PIN PG14
#endif




/**
 * @todo - Wifi setup process in case no permanent setup yet done
 * @todo - RingBuffer hack to be reviewed
 * @todo - Review DIAGS in the code and put at the right level ( ev reorder to have info at 3 )
 * @todo - check error conditions and instrument them ( buffer overruns in particular!! )
 * @todo - setup the WiFi 'shield' on any serial port / Software Serial. 
 * @todo - test Wifi/Ethernet UDP with multiple WiThrottles; build a test runner for this
 * @todo - documentation / Doxygen to be generated ...
 * @todo - add warning if all connections available are in use. i.e. the next connection opened will not go through but wait until a connection gets available
 * @todo - start on the CLI 
 * @todo - check if the number of connections is global or per [Ethernet|WiFi]Server instance ...
 */
