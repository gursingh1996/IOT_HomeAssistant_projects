#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
#define status_led BUILTIN_LED

#define SH_CP 16
#define ST_CP 14
#define DS    12
#define EN    13

#define latchPin ST_CP
#define clockPin SH_CP
#define dataPin DS

#define lightRelay              B01000000
#define lightPin                1
#define fanRelay                B00110000
#define fanPin                  2

int data = B00000000;

const char* ssid = "Tenda_E0B350";
const char* password = "meetupper430";
const char* mqtt_server = "192.168.0.108";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;


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

void writeData(int pin, int value){
  if(pin==lightPin){
    value ? data|=lightRelay : data&=~(lightRelay); 
  }
  else if(pin==fanPin){
    value ? data|=fanRelay : data&=~(fanRelay); 
  }
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("\nMessage arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  if ((char)payload[0] == 'a'){
    writeData(lightPin, 1);
    client.publish("/gurpreetRoom/light/state", "a");
  }
  else if((char)payload[0] == 'b'){
    writeData(lightPin, 0);
    client.publish("/gurpreetRoom/light/state", "b");
  }
  else if ((char)payload[0] == 'c'){
    writeData(fanPin, 1);
    client.publish("/gurpreetRoom/fan/state", "c");  
  }
  else if((char)payload[0] == 'd'){
    writeData(fanPin, 0);
    client.publish("/gurpreetRoom/fan/state", "d"); 
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "lens_5kchta2dWhdRS2AQz0TeTuhplPE";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),"mqtt-connect-user", "1234")) {
      Serial.println("connected");
      client.subscribe("/gurpreetRoom/light/command");
      client.subscribe("/gurpreetRoom/fan/command");
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
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);      //donot read the noise - 74hc595
  pinMode(SH_CP, OUTPUT);
  pinMode(ST_CP, OUTPUT);
  pinMode(DS, OUTPUT);
  delay(200);
  digitalWrite(EN, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
