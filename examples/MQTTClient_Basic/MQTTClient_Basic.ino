/****************************************************************************************************************************
  MQTTClient_Basic.ino

  For W5500 LwIP Ethernet in ESP32 (ESP32 + W5500)

  AsyncWebServer_ESP32_W5500 is a library for the LwIP Ethernet W5500 in ESP32 to run AsyncWebServer

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncWebServer_ESP32_W5500
  Licensed under GPLv3 license
 *****************************************************************************************************************************/

/*
  Basic MQTT example (without SSL!) with Authentication
  This sketch demonstrates the basic capabilities of the library.
  It connects to an MQTT server then:
  - providing username and password
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.
*/

// To remove boolean warnings caused by PubSubClient library
#define boolean     bool

#include "defines.h"

#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* mqttServer = "broker.emqx.io";        // Broker address

const char *ID        = "MQTTClient_SSL-Client";  // Name of our device, must be unique
const char *TOPIC     = "MQTT_Pub";               // Topic to subcribe to
const char *subTopic  = "MQTT_Sub";               // Topic to subcribe to

void callback(char* topic, byte* payload, unsigned int length)
{
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");

	for (unsigned int i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}

	Serial.println();
}

WiFiClient   ethClient;
PubSubClient client(mqttServer, 1883, callback, ethClient);

String data = "Hello from MQTTClient_Basic on " + String(ARDUINO_BOARD) + " with " + String(SHIELD_TYPE);
const char *pubData = data.c_str();

void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected())
	{
		Serial.print("Attempting MQTT connection to ");
		Serial.print(mqttServer);

		// Attempt to connect
		if (client.connect(ID, "try", "try"))
		{
			Serial.println("...connected");

			// Once connected, publish an announcement...
			client.publish(TOPIC, data.c_str());

			//Serial.println("Published connection message successfully!");
			//Serial.print("Subcribed to: ");
			//Serial.println(subTopic);

			client.subscribe(subTopic);
			// for loopback testing
			client.subscribe(TOPIC);
		}
		else
		{
			Serial.print("...failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");

			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void setup()
{
	// Open serial communications and wait for port to open:
	Serial.begin(115200);

	while (!Serial && millis() < 5000);

	Serial.print("\nStart MQTTClient_Basic on ");
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

	client.setServer(mqttServer, 1883);
	client.setCallback(callback);

	// Allow the hardware to sort itself out
	delay(1500);
}

#define MQTT_PUBLISH_INTERVAL_MS       5000L

unsigned long lastMsg = 0;

void loop()
{
	static unsigned long now;

	if (!client.connected())
	{
		reconnect();
	}

	// Sending Data
	now = millis();

	if (now - lastMsg > MQTT_PUBLISH_INTERVAL_MS)
	{
		lastMsg = now;

		if (!client.publish(TOPIC, pubData))
		{
			Serial.println("Message failed to send.");
		}

		Serial.print("Message Send : " + String(TOPIC) + " => ");
		Serial.println(data);
	}

	client.loop();
}
