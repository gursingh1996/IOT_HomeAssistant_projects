#include <ESP8266WiFi.h>
#include <PubSubClient.h>   //required for mqtt
#include <Espalexa.h>    //required for alexa
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

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

void lightChanged(EspalexaDevice* dev);   //callback functions //callback type, contains device pointer
void fanChanged(EspalexaDevice* dev);   //callback functions //callback type, contains device pointer

WiFiClient espClient;
PubSubClient client(espClient);

Espalexa espalexa;    //esp alexa device

EspalexaDevice* Light;
EspalexaDevice* Fan;

long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname("ESP_Bedroom");
  WiFi.mode(WIFI_STA);
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

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Light = new EspalexaDevice("Light", lightChanged, EspalexaDeviceType::onoff); //ON - OFF light
  espalexa.addDevice(Light);
  Fan = new EspalexaDevice("Fan", fanChanged, EspalexaDeviceType::onoff); //ON - OFF fan
  espalexa.addDevice(Fan);
  
  espalexa.begin();
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
  ArduinoOTA.handle();
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(WiFi.status() != WL_CONNECTED) setup_wifi();   //if wifi disconnects try reconnecting
  espalexa.loop();        //look for alexa commands
  delay(1);
}

void lightChanged(EspalexaDevice* d) {    //call back function
  if (d == nullptr) return; //this is good practice, but not required
  Serial.print("Light changed to ");
  if (d->getValue()){
    Serial.println("Light is now ON");
    writeData(lightPin, 1);
    client.publish("/gurpreetRoom/light/state", "a");
  }
  else {
    Serial.println("OFF");
    writeData(lightPin, 0);
    client.publish("/gurpreetRoom/light/state", "b");
  }
}

void fanChanged(EspalexaDevice* d) {    //call back function
  if (d == nullptr) return; //this is good practice, but not required
  Serial.print("fan changed to ");
  if (d->getValue()){
    Serial.println("Fan is now ON");
    writeData(fanPin, 1);
    client.publish("/gurpreetRoom/fan/state", "c"); 
  }
  else {
    Serial.println("OFF");
    writeData(fanPin, 0);
    client.publish("/gurpreetRoom/fan/state", "d"); 
  }
}
