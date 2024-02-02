/*

 MQTT Button LED example

 This sketch demonstrates the basic capabilities of the library.
  - connects to an MQTT server
  - subscribes to the topic "subTopic", printing out any messages it receives. It assumes the received payloads are strings not binary
  - publishs the topic "pubTopic" every 1s

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 */

#include <WiFi.h>
#include <GTimer.h>
#include <PubSubClient.h>

#define TIMER_ID                    0
#define DEVICE_NAME                 "hub8735"
#define PUBLISH_BTN_TOPIC           "btn"
#define SUBSCRIBE_LEDB_TOPIC        "led_b"

char ssid[] = "HUB8735_AP_5G";         //"Network_SSID";       // your network SSID (name)
char pass[] = "1234567890";         //"Password";           // your network password
int status = WL_IDLE_STATUS;        // Indicater of Wifi status

char mqttServer[]     = "test.mosquitto.org";
unsigned int mqttPort = 1883;
char clientId[256];
char publishBtnTopic[256];
char subscribeLEDBTopic[256];
int publishEvent = 0;
int publishEventCount = 0;
int btnState = 0;


void callback(char* topic, byte* payload, unsigned int length) {
    String str_payload = "";

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)(payload[i]));
        str_payload += (char)(payload[i]);
    }
    Serial.println();

    if(String(topic) == String(subscribeLEDBTopic))
    {
      if((str_payload == "on") || (str_payload == "ON"))
      {
        Serial.println("!!!LED_B ON!!!");
        digitalWrite(LED_B, HIGH);
      }
      else if((str_payload == "off") || (str_payload == "OFF"))
      {
        Serial.println("!!!LED_B OFF!!!");
        digitalWrite(LED_B, LOW);
      }
    }
}

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void gtimer(uint32_t data) {
    data = data;
    
    //Button detection
    if((digitalRead(PUSH_BTN) == LOW) && (!btnState))
    {
      Serial.println("Press BTN");
      publishEvent = 1;
      btnState = 1;
    }
    else if((digitalRead(PUSH_BTN) == HIGH) && (btnState))
    {
      btnState = 0;
    }
}

void reconnect() {
    //Create clientId and topics
    sprintf(clientId, "%s_%05d", DEVICE_NAME, (random()/100000));
    sprintf(publishBtnTopic, "%s/%s", clientId, PUBLISH_BTN_TOPIC);
    sprintf(subscribeLEDBTopic, "%s/%s", clientId, SUBSCRIBE_LEDB_TOPIC);

    delay(5000);

    //Show MQTT informations
    Serial.print("ClientId: ");
    Serial.println(clientId);
    Serial.print("Host: mqtt:// ");
    Serial.println(mqttServer);
    Serial.print("Port: ");
    Serial.println(mqttPort);
    Serial.println("Published Topics List==========");
    Serial.println(publishBtnTopic);
    Serial.println("Subscribed Topics List========");
    Serial.println(subscribeLEDBTopic);


    // Loop until we're reconnected
    while (!(client.connected())) {
        Serial.print("\r\nAttempting MQTT connection...");
        // Attempt to connect
        if (client.connect(clientId)) {
            Serial.println("connected");
            //Subscribe topics
            client.subscribe(subscribeLEDBTopic);
        } else {
            Serial.println("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            //Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setup() {
    //Initialize peripherals
    pinMode(LED_B, OUTPUT);
    pinMode(PUSH_BTN, INPUT_PULLUP);

    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    // wait for serial port to connect.
    while (!Serial) {
        ;
    }

    //Attempt to connect to WiFi network
    while (status != WL_CONNECTED) {
        Serial.print("\r\nAttempting to connect to SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(10000);
    }

    wifiClient.setNonBlockingMode();
    client.setServer(mqttServer, mqttPort);
    client.setCallback(callback);

    //Allow Hardware to sort itself out
    delay(1500);

    GTimer.begin(TIMER_ID, (1 * 1000 * 1000), gtimer);
}

void loop() {
    char publishPayload[256];

    if (!(client.connected())) {
        reconnect();
    } else if (publishEvent == 1) {
      publishEventCount++;
      sprintf(publishPayload, "!!!BTN click %d !!!", publishEventCount);
      client.publish(publishBtnTopic, publishPayload);
      publishEvent = 0;
    }
    client.loop();
}
