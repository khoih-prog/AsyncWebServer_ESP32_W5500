/****************************************************************************************************************************
  esp_eth_w5500.h

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

  Version: 1.6.2

  Version Modified By   Date      Comments
  ------- -----------  ---------- -----------
  1.6.2   K Hoang      30/11/2022 Initial porting for ENC28J60 + ESP32. Sync with AsyncWebServer_WT32_ETH01 v1.6.2
 *****************************************************************************************************************************/

// Copyright 2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////////////

#include "esp_eth_phy.h"
#include "esp_eth_mac.h"
#include "driver/spi_master.h"

////////////////////////////////////////

#define CS_HOLD_TIME_MIN_NS     210

////////////////////////////////////////

/*
  // From tools/sdk/esp32/include/esp_eth/include/esp_eth_mac.h

  typedef struct
  {
  void *spi_hdl;     //!< Handle of SPI device driver
  int int_gpio_num;  //!< Interrupt GPIO number
  } eth_w5500_config_t;


  #define ETH_W5500_DEFAULT_CONFIG(spi_device) \
  {                                            \
    .spi_hdl = spi_device,                   \
    .int_gpio_num = 4,                       \
  }

*/


////////////////////////////////////////

/**
   @brief Compute amount of SPI bit-cycles the CS should stay active after the transmission
          to meet w5500 CS Hold Time specification.

   @param clock_speed_mhz SPI Clock frequency in MHz (valid range is <1, 20>)
   @return uint8_t
*/
static inline uint8_t w5500_cal_spi_cs_hold_time(int clock_speed_mhz)
{
  if (clock_speed_mhz <= 0 || clock_speed_mhz > 20)
  {
    return 0;
  }

  int temp = clock_speed_mhz * CS_HOLD_TIME_MIN_NS;
  uint8_t cs_posttrans = temp / 1000;

  if (temp % 1000)
  {
    cs_posttrans += 1;
  }

  return cs_posttrans;
}

////////////////////////////////////////

/**
  @brief Create w5500 Ethernet MAC instance

  @param[in] w5500_config: w5500 specific configuration
  @param[in] mac_config: Ethernet MAC configuration

  @return
       - instance: create MAC instance successfully
       - NULL: create MAC instance failed because some error occurred
*/
esp_eth_mac_t *esp_eth_mac_new_w5500(const eth_w5500_config_t *w5500_config,
                                     const eth_mac_config_t *mac_config);

////////////////////////////////////////

/**
  @brief Create a PHY instance of w5500

  @param[in] config: configuration of PHY

  @return
       - instance: create PHY instance successfully
       - NULL: create PHY instance failed because some error occurred
*/
esp_eth_phy_t *esp_eth_phy_new_w5500(const eth_phy_config_t *config);

////////////////////////////////////////

// todo: the below functions should be accessed through ioctl in the future
/**
   @brief Set w5500 Duplex mode. It sets Duplex mode first to the PHY and then
          MAC is set based on what PHY indicates.

   @param phy w5500 PHY Handle
   @param duplex Duplex mode

   @return esp_err_t
            - ESP_OK when PHY registers were correctly written.
*/
esp_err_t w5500_set_phy_duplex(esp_eth_phy_t *phy, eth_duplex_t duplex);

////////////////////////////////////////

#ifdef __cplusplus
}
#endif
