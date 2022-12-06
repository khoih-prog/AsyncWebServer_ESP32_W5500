/****************************************************************************************************************************
  MQTT_ThingStream.ino

  For W5500 LwIP Ethernet in ESP32 (ESP32 + W5500)

  AsyncWebServer_ESP32_W5500 is a library for the LwIP Ethernet W5500 in ESP32 to run AsyncWebServer

  Based on and modified from ESPAsyncWebServer (https://github.com/me-no-dev/ESPAsyncWebServer)
  Built by Khoi Hoang https://github.com/khoih-prog/AsyncWebServer_ESP32_W5500
  Licensed under GPLv3 license
 *****************************************************************************************************************************/

/*
  Basic MQTT example (without SSL!)
  This sketch demonstrates the basic capabilities of the library.
  It connects to an MQTT server then:
  - publishes {Hello from MQTTClient_SSL on RP2040W} to the topic [RP2040W_Pub]
  - subscribes to the topic [RP2040W_Sub], printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  You will need to populate "certificates.h" with your trust anchors
  (see https://github.com/OPEnSLab-OSU/SSLClient/blob/master/TrustAnchors.md)
  and my_cert/my_key with your certificate/private key pair
  (see https://github.com/OPEnSLab-OSU/SSLClient#mtls).
*/

// To remove boolean warnings caused by PubSubClient library
#define boolean     bool

#include "defines.h"

#include <PubSubClient.h>

const char my_cert[]  = "FIXME";
const char my_key[]   = "FIXME";

#define USING_THINGSTREAM_IO      false   //true

#if USING_THINGSTREAM_IO

	const char *MQTT_PREFIX_TOPIC   = "esp32-sniffer/";
	const char *MQTT_ANNOUNCE_TOPIC = "/status";
	const char *MQTT_CONTROL_TOPIC  = "/control";
	const char *MQTT_BLE_TOPIC      = "/ble";


	// GOT FROM ThingsStream!
	const char *MQTT_SERVER     = "mqtt.thingstream.io";
	const char *MQTT_USER       = "MQTT_USER";
	const char *MQTT_PASS       = "MQTT_PASS";
	const char *MQTT_CLIENT_ID  = "MQTT_CLIENT_ID";

	String topic    = MQTT_PREFIX_TOPIC + String("12345678") + MQTT_BLE_TOPIC;
	String subTopic = MQTT_PREFIX_TOPIC + String("12345678") + MQTT_BLE_TOPIC;

#else

	const char* MQTT_SERVER = "broker.emqx.io";           // Broker address

	const char*  ID         = "MQTT_ThingStream";         // Name of our device, must be unique
	String      topic       = "RP2040W_Pub";              // Topic to publish
	String      subTopic    = "RP2040W_Sub";              // Topic to subcribe to

#endif

void mqtt_receive_callback(char* topic, byte* payload, unsigned int length);

const int   MQTT_PORT           = 1883; //if you use SSL //1883 no SSL

unsigned long lastMsg = 0;

// Initialize the SSL client library
// Arguments: ethClient, our trust anchors

WiFiClient   ethClient;
PubSubClient client(MQTT_SERVER, MQTT_PORT, mqtt_receive_callback, ethClient);

String data         = "Hello from MQTT_ThingStream on " + String(ARDUINO_BOARD) + " with " + String(SHIELD_TYPE);
const char *pubData = data.c_str();

/*
   Called whenever a payload is received from a subscribed MQTT topic
*/
void mqtt_receive_callback(char* topic, byte* payload, unsigned int length)
{
	Serial.print("MQTT Message receive [");
	Serial.print(topic);
	Serial.print("] ");

	for (unsigned int i = 0; i < length; i++)
	{
		Serial.print((char)payload[i]);
	}

	Serial.println();
}

void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected())
	{
		Serial.print("Attempting MQTT connection to ");
		Serial.println(MQTT_SERVER);

		// Attempt to connect

#if USING_THINGSTREAM_IO
		int connect_status = client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS, topic.c_str(), 2, false, "");
#else
		int connect_status = client.connect(ID);
#endif

		if (connect_status)
		{
			Serial.println("...connected");

			// Once connected, publish an announcement...
			client.publish(topic.c_str(), data.c_str());

			Serial.println("Published connection message successfully!");

			Serial.print("Subcribed to: ");
			Serial.println(subTopic);

			// This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
			//ethClientSSL.flush();
			// ... and resubscribe
			client.subscribe(subTopic.c_str());
			// for loopback testing
			client.subscribe(topic.c_str());
			// This is a workaround to address https://github.com/OPEnSLab-OSU/SSLClient/issues/9
			//ethClientSSL.flush();
		}
		else
		{
			Serial.print("failed, rc=");
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

	Serial.print("\nStart MQTT_ThingStream on ");
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

	// Note - the default maximum packet size is 256 bytes. If the
	// combined length of clientId, username and password exceed this use the
	// following to increase the buffer size:
	//client.setBufferSize(256);

	Serial.println("***************************************");
	Serial.println(topic);
	Serial.println("***************************************");
}

#define MQTT_PUBLISH_INTERVAL_MS      5000L

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

		if (!client.publish(topic.c_str(), pubData))
		{
			Serial.println("Message failed to send.");
		}

		Serial.print("MQTT Message Send : " + topic + " => ");
		Serial.println(data);
	}

	client.loop();
}
