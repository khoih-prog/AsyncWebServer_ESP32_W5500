/****************************************************************************************************************************
  AsyncMultiWebServer_ESP32_W5500.ino

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

unsigned int    analogReadPin []  = { 12, 13, 14 };

#define BUFFER_SIZE       500

#define HTTP_PORT1        8080
#define HTTP_PORT2        8081
#define HTTP_PORT3        8082

AsyncWebServer* server1;
AsyncWebServer* server2;
AsyncWebServer* server3;

AsyncWebServer*  multiServer  []  = { server1, server2, server3 };
uint16_t        http_port     []  = { HTTP_PORT1, HTTP_PORT2, HTTP_PORT3 };

#define NUM_SERVERS     ( sizeof(multiServer) / sizeof(AsyncWebServer*) )

unsigned int serverIndex;

String createBuffer()
{
  char temp[BUFFER_SIZE];

  memset(temp, 0, sizeof(temp));

  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  int day = hr / 24;

  snprintf(temp, BUFFER_SIZE - 1,
           "<html>\
<head>\
<meta http-equiv='refresh' content='5'/>\
<title>%s</title>\
<style>\
body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
</style>\
</head>\
<body>\
<h1>Hello from %s</h1>\
<h2>running AsyncMultiWebServer_ESP32_W5500</h2>\
<h3>on %s</h3>\
<h3>Uptime: %d d %02d:%02d:%02d</h3>\
</body>\
</html>", ARDUINO_BOARD, ARDUINO_BOARD, SHIELD_TYPE, day, hr, min % 60, sec % 60);

  return temp;
}


void handleRoot(AsyncWebServerRequest * request)
{
  String message = createBuffer();
  request->send(200, F("text/html"), message);
}

String createNotFoundBuffer(AsyncWebServerRequest * request)
{
  String message;

  message.reserve(500);

  message = F("File Not Found\n\n");

  message += F("URI: ");
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? F("GET") : F("POST");
  message += F("\nArguments: ");
  message += request->args();
  message += F("\n");

  for (uint8_t i = 0; i < request->args(); i++)
  {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  return message;
}

void handleNotFound(AsyncWebServerRequest * request)
{
  String message = createNotFoundBuffer(request);
  request->send(404, F("text/plain"), message);
}

void setup()
{
  Serial.begin(115200);

  while (!Serial && millis() < 5000);

  delay(200);

  Serial.print(F("\nStart AsyncMultiWebServer_ESP32_W5500 on "));
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

  Serial.print("\nConnected to network. IP = ");
  Serial.println(ETH.localIP());

  for (serverIndex = 0; serverIndex < NUM_SERVERS; serverIndex++)
  {
    multiServer[serverIndex] = new AsyncWebServer(http_port[serverIndex]);

    if (multiServer[serverIndex])
    {
      Serial.printf("Initialize multiServer OK, serverIndex = %d, port = %d\n", serverIndex, http_port[serverIndex]);
    }
    else
    {
      Serial.printf("Error initialize multiServer, serverIndex = %d\n", serverIndex);

      while (1);
    }

    multiServer[serverIndex]->on("/", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      handleRoot(request);
    });

    multiServer[serverIndex]->on("/hello", HTTP_GET, [](AsyncWebServerRequest * request)
    {
      String message = F("Hello from AsyncMultiWebServer_ESP32_W5500 using W5500 Ethernet, running on ");
      message       += ARDUINO_BOARD;

      request->send(200, "text/plain", message);
    });

    multiServer[serverIndex]->onNotFound([](AsyncWebServerRequest * request)
    {
      handleNotFound(request);
    });

    multiServer[serverIndex]->begin();

    Serial.printf("HTTP server started at ports %d\n", http_port[serverIndex]);
  }
}

void loop()
{
}
