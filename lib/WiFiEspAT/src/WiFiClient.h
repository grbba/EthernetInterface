/*
  This file is part of the WiFiEspAT library for Arduino
  https://github.com/jandrassy/WiFiEspAT
  Copyright 2019 Juraj Andrassy

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _WIFICLIENT_H_
#define _WIFICLIENT_H_

#include <Client.h>
#include "WiFiEspAtSharedBuffStreamPtr.h"
#include "WiFiEspAtConfig.h"

enum WiFiTcpState {
  WIFI_CLOSED      = 0,
  WIFI_LISTEN      = 1,
  WIFI_SYN_SENT    = 2,
  WIFI_SYN_RCVD    = 3,
  WIFI_ESTABLISHED = 4,
  WIFI_FIN_WAIT_1  = 5,
  WIFI_FIN_WAIT_2  = 6,
  WIFI_CLOSE_WAIT  = 7,
  WIFI_CLOSING     = 8,
  WIFI_LAST_ACK    = 9,
  WIFI_TIME_WAIT   = 10
};

class WiFiServer;

class WiFiClient : public Client {

  friend WiFiServer;
  WiFiClient(uint8_t linkId);

public:
  WiFiClient();

  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  int connectSSL(IPAddress ip, uint16_t port);
  int connectSSL(const char *host, uint16_t port);
  virtual void stop();
          void abort();

  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual void flush();

  size_t write(Stream& file);
  size_t write(SendCallbackFnc callback);

  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();

  virtual operator bool();
  virtual uint8_t connected();
  uint8_t status();

  // remoteIP and the ports are not stored
  // every call to these functions retrieves the value from AT firmware
  IPAddress remoteIP();
  uint16_t remotePort();
  uint16_t localPort();

  using Print::write;

private:
  int connect(bool ssl, IPAddress ip, uint16_t port);
  int connect(bool ssl, const char *host, uint16_t port);

  WiFiEspAtSharedBuffStreamPtr stream;

};

#endif
