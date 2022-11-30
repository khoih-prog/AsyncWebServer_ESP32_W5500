/****************************************************************************************************************************
  AsyncSimpleServer_ESP32_W5500.ino

  For W5500 LwIP Ethernet in ESP32 (ESP32 + W5500)

  AsyncWebServer_ESP32_W5500 is a library for the LwIP Ethernet W5500 in ESP32 to run AsyncWebServer

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncWebServer_ESP32_W5500
  Licensed under GPLv3 license
 *****************************************************************************************************************************/

#if !( defined(ESP32) )
  #error This code is designed for (ESP32 + W5500) to run on ESP32 platform! Please check your Tools->Board setting.
#endif

#include <Arduino.h>

#define _ASYNC_WEBSERVER_LOGLEVEL_       2

// Enter a MAC address and IP address for your controller below.
#define NUMBER_OF_MAC      20

byte mac[][NUMBER_OF_MAC] =
{
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x02 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x03 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x04 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x05 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x06 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x07 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x08 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x09 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x0A },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0B },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x0C },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0D },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x0E },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x0F },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x10 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x11 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x12 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x13 },
  { 0xDE, 0xAD, 0xBE, 0xEF, 0xBE, 0x14 },
};

// Select the IP address according to your local network
IPAddress myIP(192, 168, 2, 232);
IPAddress myGW(192, 168, 2, 1);
IPAddress mySN(255, 255, 255, 0);

// Google DNS Server IP
IPAddress myDNS(8, 8, 8, 8);

//////////////////////////////////////////////////////////

// Optional values to override default settings
// Don't change unless you know what you're doing
//#define ETH_SPI_HOST        SPI3_HOST
//#define SPI_CLOCK_MHZ       25

// Must connect INT to GPIOxx or not working
//#define INT_GPIO            4

//#define MISO_GPIO           19
//#define MOSI_GPIO           23
//#define SCK_GPIO            18
//#define CS_GPIO             5

//////////////////////////////////////////////////////////

#include <AsyncTCP.h>

#include <AsyncWebServer_ESP32_W5500.h>

AsyncWebServer server(80);

const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  Serial.print(F("\nStart AsyncSimpleServer_ESP32_W5500 on "));
  Serial.print(ARDUINO_BOARD);
  Serial.print(F(" with "));
  Serial.println(SHIELD_TYPE);
  Serial.println(ASYNC_WEBSERVER_ESP32_W5500_VERSION);

  AWS_LOGWARN(F("Default SPI pinout:"));
  AWS_LOGWARN1(F("SPI_HOST:"), ETH_SPI_HOST);
  AWS_LOGWARN1(F("MOSI:"), MOSI_GPIO);
  AWS_LOGWARN1(F("MISO:"), MISO_GPIO);
  AWS_LOGWARN1(F("SCK:"),  SCK_GPIO);
  AWS_LOGWARN1(F("CS:"),   CS_GPIO);
  AWS_LOGWARN1(F("INT:"),  INT_GPIO);
  AWS_LOGWARN1(F("SPI Clock (MHz):"), SPI_CLOCK_MHZ);
  AWS_LOGWARN(F("========================="));

  ///////////////////////////////////

  // To be called before ETH.begin()
  ESP32_W5500_onEvent();

  // start the ethernet connection and the server:
  // Use DHCP dynamic IP and random mac
  uint16_t index = millis() % NUMBER_OF_MAC;

  //bool begin(int MISO_GPIO, int MOSI_GPIO, int SCLK_GPIO, int CS_GPIO, int INT_GPIO, int SPI_CLOCK_MHZ,
  //           int SPI_HOST, uint8_t *W5500_Mac = W5500_Default_Mac);
  //ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST );
  ETH.begin( MISO_GPIO, MOSI_GPIO, SCK_GPIO, CS_GPIO, INT_GPIO, SPI_CLOCK_MHZ, ETH_SPI_HOST, mac[index] );

  // Static IP, leave without this line to get IP via DHCP
  //bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1 = 0, IPAddress dns2 = 0);
  //ETH.config(myIP, myGW, mySN, myDNS);

  ESP32_W5500_waitForConnect();

  ///////////////////////////////////

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(200, "text/plain", "Hello, world from AsyncSimpleServer_ESP32_W5500");
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request)
  {
    String message;

    if (request->hasParam(PARAM_MESSAGE))
    {
      message = request->getParam(PARAM_MESSAGE)->value();
    }
    else
    {
      message = "No message sent";
    }

    request->send(200, "text/plain", "Hello, GET: " + message);
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/post", HTTP_POST, [](AsyncWebServerRequest * request)
  {
    String message;

    if (request->hasParam(PARAM_MESSAGE, true))
    {
      message = request->getParam(PARAM_MESSAGE, true)->value();
    }
    else
    {
      message = "No message sent";
    }

    request->send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(notFound);

  server.begin();
}

void loop()
{
}
