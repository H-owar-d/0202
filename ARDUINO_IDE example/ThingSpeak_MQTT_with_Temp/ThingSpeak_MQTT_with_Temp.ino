/*
 ThingSpeak MQTT with Temp example
  - connects to ThingSpeak MQTT broker
  - publishes temperature & humidity every 5s
 */

#include <WiFi.h>
#include <GTimer.h>
#include <PubSubClient.h>
#include "DHT.h"

#define TIMER_ID                    0
#define DHTTYPE                     DHT11   //Select DHT11

char ssid[] = "HUB8735_AP_5G"; // your network SSID (name)
char pass[] = "1234567890";     // your network password
int status = WL_IDLE_STATUS;  // Indicater of Wifi status

char mqttServer[]     = "mqtt3.thingspeak.com";
int mqttPort          = 1883;
char TSClientId[]     = "***********************";        //your MQTT client id from thingspeak
char TSClientUser[]   = "***********************";        //your MQTT client username from thingspeak
char TSClientPass[]   = "***********************";        //your MQTT client password from thingspeak
char TSPublishTopic[] = "channels/<ChannelID>/publish";  //your channel id from thingspeak

int dhtPin = 20;
int dhtPub = 0;
DHT dht(dhtPin, DHTTYPE);

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)(payload[i]));
    }
    Serial.println();
}

WiFiClient wifiClient;
PubSubClient client(mqttServer, mqttPort, callback, wifiClient);

void gtimer_hdl(uint32_t data) {
  data = data;

  if (dhtPub == 0) {
    dhtPub = 1;
  }

}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  // wait for serial port to connect.
  while (!Serial) {
      ;
  }

  //Show MQTT informations
  Serial.print("ClientId: ");
  Serial.println(TSClientId);
  Serial.print("ClientUserName: ");
  Serial.println(TSClientUser);
  Serial.print("ClientPassword: ");
  Serial.println(TSClientPass);
  Serial.print("Host: mqtt:// ");
  Serial.println(mqttServer);
  Serial.print("Port: ");
  Serial.println(mqttPort);
  Serial.println("Published Topics List==========");
  Serial.println(TSPublishTopic);

  //Attempt to connect to WiFi network
  while (status != WL_CONNECTED) {
      Serial.print("\r\nAttempting to connect to SSID: ");
      Serial.println(ssid);
      // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
      status = WiFi.begin(ssid, pass);
    
      // wait 10 seconds for connection:
      delay(10000);
  }

  // Note - the default maximum packet size is 512 bytes. If the
  // combined length of TSClientId, username and password exceed this,
  // you will need to increase the value of MQTT_MAX_PACKET_SIZE in
  // PubSubClient.h
  wifiClient.setNonBlockingMode();
  if (client.connect(TSClientId, TSClientUser, TSClientPass)) {
      Serial.println("Connected");
      client.subscribe(TSPublishTopic);
  }

  GTimer.begin(TIMER_ID, (5 * 1000 * 1000), gtimer_hdl);
  dht.begin();
}

void loop() {
    char TSPublishPayload[256];
    //String TSPublishPayload = "";

    client.loop();
    if(dhtPub)
    {
      dhtPub = 0;

      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(t) || isnan(f)) {
          Serial.println(F("Failed to read from DHT sensor!"));
          return;
      }

      Serial.print(F("Humidity: "));
      Serial.print(h);
      Serial.print(F("%  Temperature: "));
      Serial.print(t);
      Serial.print(F("°C "));
      sprintf(TSPublishPayload, "field1=%f&field2=%f", h, t);

      Serial.println("Publish:");
      Serial.println(TSPublishTopic);
      Serial.println(TSPublishPayload);
      client.publish(TSPublishTopic, TSPublishPayload);
    }
}
