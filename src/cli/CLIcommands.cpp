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

#include "../NetworkDiag.h"
#include "../NetworkConfig.h"
#include "../Transport.h"


#ifdef CLI_ENABLED

#include "CLIcommands.h"
#include "LoginShell.h"
#include "Shell.h"

#endif

int ledPin = 13;

bool CLI::cliConnected = false;
Connection* CLI::_cliConnection = nullptr;

extern bool diagNetwork;
extern uint8_t diagNetworkClient;

void cmdLed(Shell &shell, int argc, const ShellArguments &argv)
{
    if (argc > 1 && !strcmp(argv[1], "on"))
        digitalWrite(ledPin, HIGH);
    else
        digitalWrite(ledPin, LOW);
}

void cmdAttach(Shell &shell, int argc, const ShellArguments &argv)
{
    Connection* cc = CLI::getCLIconnection();
    INFO(F("Handling Telnet attach on client [%x]"), cc->client);
    NetworkDiag::diagSerial =  cc->client;
    diagNetwork = true;
    diagNetworkClient = cc->id;
}

ShellCommand(led, "Turns the status LED on or off", cmdLed);
ShellCommand(attach, "Instructs the CommandStation to send diagnostis / messages etc to this terminal", cmdAttach);

// above is a short cut for 
// static char const shell_id_led[] PROGMEM = "led"; 
// static char const shell_help_led[] PROGMEM = "Turns the status LED on or off"; 
// static ShellCommandInfo const shell_info_led PROGMEM = { shell_id_led, shell_help_led, (cmdLed) }; 
// static ShellCommandRegister shell_cmd_led(&shell_info_led)