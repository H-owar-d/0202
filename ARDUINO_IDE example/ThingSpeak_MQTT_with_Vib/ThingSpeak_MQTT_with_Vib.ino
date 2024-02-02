/*
 ThingSpeak MQTT with Vib example
  - connects to ThingSpeak MQTT broker
  - publishes "field2=1" after vibrating

 */

#include <WiFi.h>
#include <GTimer.h>
#include <PubSubClient.h>

#define TIMER_ID                    0

char ssid[] = "HUB8735_AP_5G"; // your network SSID (name)
char pass[] = "1234567890";     // your network password
int status = WL_IDLE_STATUS;  // Indicater of Wifi status

char mqttServer[]     = "mqtt3.thingspeak.com";
int mqttPort          = 1883;
char TSClientId[]     = "***********************";        //your MQTT client id from thingspeak
char TSClientUser[]   = "***********************";        //your MQTT client username from thingspeak
char TSClientPass[]   = "***********************";        //your MQTT client password from thingspeak
char TSPublishTopic[] = "channels/<ChannelID>/publish";  //your channel id from thingspeak

int vibPin = 21;
int vibCounter = 0;
int vibState = 0;
int vibEn = 0;
int vibPub = 0;

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
  int state = digitalRead(vibPin);
  data = data;

  if ((vibState != state) && (!vibEn)) {
    vibState = state;
    vibEn = 1;
    vibPub = 1;
    vibCounter = 0;
    Serial.println("!!!Vib!!!");
  } else if ((vibState == state) && (vibEn)) {
    vibCounter++;

    if (vibCounter >= 50) {
      vibEn = 0;
      Serial.println("!!!Stop!!!");
    }
  } else {
    vibCounter = 0;
  }
  vibState = state;
}

void setup() {
  pinMode(vibPin, INPUT);

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

  vibState = digitalRead(vibPin);
  GTimer.begin(TIMER_ID, (1 * 100 * 1000), gtimer_hdl);
}

void loop() {
    client.loop();
    if((vibEn) && (vibPub))
    {
      Serial.println("Publish:");
      Serial.println(TSPublishTopic);
      client.publish(TSPublishTopic, "field1=1");
      vibPub = 0;
    }
}
