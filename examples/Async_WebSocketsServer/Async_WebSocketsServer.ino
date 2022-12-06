/****************************************************************************************************************************
  Async_WebSocketsServer.ino

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

#define _ASYNC_WEBSERVER_LOGLEVEL_       4

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

#include <AsyncWebServer_ESP32_W5500.h>

#include "webpage.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if (type == WS_EVT_CONNECT)
	{
		Serial.printf("ws[Server: %s][ClientID: %u] WSClient connected\n", server->url(), client->id());
		client->text("Hello from RP2040W Server");
	}
	else if (type == WS_EVT_DISCONNECT)
	{
		Serial.printf("ws[Server: %s][ClientID: %u] WSClient disconnected\n", server->url(), client->id());
	}
	else if (type == WS_EVT_ERROR)
	{
		//error was received from the other end
		Serial.printf("ws[Server: %s][ClientID: %u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
	}
	else if (type == WS_EVT_PONG)
	{
		//pong message was received (in response to a ping request maybe)
		Serial.printf("ws[Server: %s][ClientID: %u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char*)data : "");
	}
	else if (type == WS_EVT_DATA)
	{
		//data packet
		AwsFrameInfo * info = (AwsFrameInfo*)arg;

		if (info->final && info->index == 0 && info->len == len)
		{
			//the whole message is in a single frame and we got all of it's data
			Serial.printf("ws[Server: %s][ClientID: %u] %s-message[len: %llu]: ", server->url(), client->id(),
			              (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

			if (info->opcode == WS_TEXT)
			{
				data[len] = 0;
				Serial.printf("%s\n", (char*)data);
			}
			else
			{
				for (size_t i = 0; i < info->len; i++)
				{
					Serial.printf("%02x ", data[i]);
				}

				Serial.printf("\n");
			}

			if (info->opcode == WS_TEXT)
				client->text("Got your text message");
			else
				client->binary("Got your binary message");
		}
		else
		{
			//message is comprised of multiple frames or the frame is split into multiple packets
			if (info->index == 0)
			{
				if (info->num == 0)
				{
					Serial.printf("ws[Server: %s][ClientID: %u] %s-message start\n", server->url(), client->id(),
					              (info->message_opcode == WS_TEXT) ? "text" : "binary");
				}

				Serial.printf("ws[Server: %s][ClientID: %u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
			}

			Serial.printf("ws[Server: %s][ClientID: %u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(),
			              info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

			if (info->message_opcode == WS_TEXT)
			{
				data[len] = 0;
				Serial.printf("%s\n", (char*)data);
			}
			else
			{
				for (size_t i = 0; i < len; i++)
				{
					Serial.printf("%02x ", data[i]);
				}

				Serial.printf("\n");
			}

			if ((info->index + len) == info->len)
			{
				Serial.printf("ws[Server: %s][ClientID: %u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);

				if (info->final)
				{
					Serial.printf("ws[Server: %s][ClientID: %u] %s-message end\n", server->url(), client->id(),
					              (info->message_opcode == WS_TEXT) ? "text" : "binary");

					if (info->message_opcode == WS_TEXT)
						client->text("I got your text message");
					else
						client->binary("I got your binary message");
				}
			}
		}
	}
}

void handleRoot(AsyncWebServerRequest *request)
{
	request->send(200, "text/html", webpageCont);
}


void setup()
{
	Serial.begin(115200);

	while (!Serial && millis() < 5000);

	delay(200);

	Serial.print("\nStarting Async_WebSocketsServer on ");
	Serial.print(ARDUINO_BOARD);
  Serial.print(" with ");
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

	ws.onEvent(onWsEvent);
	server.addHandler(&ws);

	server.on("/", handleRoot);
	server.begin();
}

void loop()
{
}
