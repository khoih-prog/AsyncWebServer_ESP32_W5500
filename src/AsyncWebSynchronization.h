/****************************************************************************************************************************
  AsyncWebSynchronization.h - Dead simple Ethernet AsyncWebServer.

  For W5500 LwIP Ethernet in ESP32 (ESP32 + W5500)

  AsyncWebServer_ESP32_W5500 is a library for the LwIP Ethernet W5500 in ESP32 to run AsyncWebServer

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncWebServer_ESP32_W5500
  Licensed under GPLv3 license

  Original author: Hristo Gochkov

  Copyright (c) 2016 Hristo Gochkov. All rights reserved.

  This library is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License along with this library;
  if not, write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Version: 1.6.4

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.6.2   K Hoang      30/11/2022 Initial porting for ENC28J60 + ESP32. Sync with AsyncWebServer_WT32_ETH01 v1.6.2
  1.6.3   K Hoang      05/12/2022 Add Async_WebSocketsServer, MQTT examples
  1.6.4   K Hoang      23/12/2022 Remove unused variable to avoid compiler warning and error
 *****************************************************************************************************************************/

#ifndef ASYNCWEBSYNCHRONIZATION_H_
#define ASYNCWEBSYNCHRONIZATION_H_

// Synchronisation is only available on ESP32, as the ESP8266 isn't using FreeRTOS by default

#include "AsyncWebServer_ESP32_W5500.h"

/////////////////////////////////////////////////

// This is the ESP32 version of the Sync Lock, using the FreeRTOS Semaphore
class AsyncWebLock
{
  private:
    SemaphoreHandle_t _lock;
    mutable void *_lockedBy;

  public:
    AsyncWebLock()
    {
      _lock = xSemaphoreCreateBinary();
      _lockedBy = NULL;
      xSemaphoreGive(_lock);
    }

    /////////////////////////////////////////////////

    ~AsyncWebLock()
    {
      vSemaphoreDelete(_lock);
    }

    /////////////////////////////////////////////////

    bool lock() const
    {
      extern void *pxCurrentTCB;

      if (_lockedBy != pxCurrentTCB)
      {
        xSemaphoreTake(_lock, portMAX_DELAY);
        _lockedBy = pxCurrentTCB;

        return true;
      }

      return false;
    }

    /////////////////////////////////////////////////

    void unlock() const
    {
      _lockedBy = NULL;
      xSemaphoreGive(_lock);
    }
};

/////////////////////////////////////////////////

class AsyncWebLockGuard
{
  private:
    const AsyncWebLock *_lock;

  public:
    AsyncWebLockGuard(const AsyncWebLock &l)
    {
      if (l.lock())
      {
        _lock = &l;
      }
      else
      {
        _lock = NULL;
      }
    }

    /////////////////////////////////////////////////

    ~AsyncWebLockGuard()
    {
      if (_lock)
      {
        _lock->unlock();
      }
    }
};

/////////////////////////////////////////////////

#endif // ASYNCWEBSYNCHRONIZATION_H_
