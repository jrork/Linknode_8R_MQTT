/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* ssid = "SSID";
const char* password = "password";
//const char* mqtt_server = "broker.mqtt-dashboard.com";
const IPAddress mqtt_server(192, 168, 1, 50);

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
//-------r4 values
byte relay[] = {12,13,14,16};
char* payloadChars;
int numberOfInputs = 4;
String device = "LinkNode-R4-";
char* device_topic_label = "r4";
char status_topic[] = "/home/switches/r4/status";
char connect_confirmation_message[] = "Linknode R4 Connected";

//-------r8 values
//byte relay[] = {4,5,10,12,13,14,15,16};
//char* payloadChars;
//int numberOfInputs = 8;
//String device = "LinkNode-R8-";
//char* device_topic_label = "r8";
//char status_topic[] = "/home/switches/r8/status";
//char connect_confirmation_message[] = "Linknode R8 Connected";

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String payload_string;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
//Print out the payload
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payload_string += (String)(char)payload[i]; //this is new
  }
  Serial.println();
 
   int selection = (String(topic)).substring(18).toInt();
      
  if (payload_string.equals("on")){
    String status_string = device;
    status_string += "Turing on switch number ";
    status_string += (String)selection;
    Serial.print(status_string);
    Serial.println();
    char buf[status_string.length()+1];
    status_string.toCharArray(buf, status_string.length()+1);
    client.publish(status_topic, buf);
    digitalWrite(relay[selection-1], HIGH);
  
  } else if (payload_string.equals("off")){
    String status_string = device;
    status_string += "Turing off switch number ";
    status_string += (String)selection;
    Serial.print(status_string);
    Serial.println();
    char buf[status_string.length()+1];
    status_string.toCharArray(buf, status_string.length()+1);
    client.publish(status_topic, buf);
    digitalWrite(relay[selection-1], LOW);
  }
    else {
      Serial.print("Invalid Request");
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = device;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println(connect_confirmation_message);
      // Once connected, publish an announcement...
      client.publish(status_topic, connect_confirmation_message);
      // ... and resubscribe
      for (int i=1; i<numberOfInputs+1; i++){
        String subTopic = "/home/switches/";
        subTopic += device_topic_label;
        subTopic += "/";
        subTopic += String(i);
        char buf[subTopic.length()+1];
        subTopic.toCharArray(buf, subTopic.length()+1);
        client.subscribe(buf);
      }

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  for(int i=0; i<numberOfInputs; i++){
    pinMode(relay[i], OUTPUT);
  } 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
