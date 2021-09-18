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

#define drinkingWaterMotorRelay B01000000
#define drinkingWaterMotorPin   1
#define otherWaterMotorRelay    B00100000
#define otherWaterMotorPin      2
#define light                   B00010000
#define lightPin                3

int data = B00000000;

const char* ssid = "himanshu";
const char* password = "#qwerty12345";
const char* mqtt_server = "192.168.1.105";

WiFiClient espClient;
PubSubClient client(espClient);

IPAddress ip(192, 168, 1, 217); 
IPAddress gateway_dns(192, 168, 1, 1);

long lastMsg = 0;
char msg[50];
int value = 0;


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  //WiFi.config(ip, gateway_dns, gateway_dns); 
  
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
  if(pin==drinkingWaterMotorPin){
    value ? data|=drinkingWaterMotorRelay : data&=~(drinkingWaterMotorRelay); 
  }
  else if(pin==otherWaterMotorPin){
    value ? data|=otherWaterMotorRelay : data&=~(otherWaterMotorRelay); 
  }
  else if(pin==lightPin){
    value ? data|=light : data&=~(light); 
  }
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, data);
  digitalWrite(latchPin, HIGH);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  if ((char)payload[0] == 'a')
    writeData(drinkingWaterMotorPin, 1);
  else if((char)payload[0] == 'b')
    writeData(drinkingWaterMotorPin, 0);
  if ((char)payload[0] == 'c')
    writeData(otherWaterMotorPin, 1);
  else if((char)payload[0] == 'd')
    writeData(otherWaterMotorPin, 0);
  if ((char)payload[0] == 'e')
    writeData(lightPin, 1);
  else if((char)payload[0] == 'f')
    writeData(lightPin, 0);  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "lens_5kchta2dWhdRS2AQz0TeTuhplPE";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),"openhabian", "openhabian")) {
      Serial.println("connected");
      client.subscribe("/home/DWM");
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
