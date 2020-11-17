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

#ifndef CLIcommands_h
#define CLICommands_h
#include "Transport.h"

class CLI
{

private:
    static Connection* _cliConnection;
    static bool cliConnected; 

public:
    CLI() = default;
    CLI(CLI &&) = default;
    CLI(const CLI &) = default;
    CLI &operator=(CLI &&) = default;
    CLI &operator=(const CLI &) = default;
    ~CLI() = default;

    static bool connected(){
        return cliConnected;
    }

    static void connect(Connection *c){
        if( cliConnected ) return;
        _cliConnection = c;
        cliConnected = true;
    }
    
    static Connection *getCLIconnection() {
        if(cliConnected) return _cliConnection;
        return nullptr;
    }
};

#endif