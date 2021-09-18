#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
#define status_led BUILTIN_LED

#define lockPin    14

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

void writeData(int pin){
  digitalWrite(lockPin, HIGH);
  delay(200);
  digitalWrite(lockPin, LOW);
  client.publish("/doorLock", "b");
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  if ((char)payload[0] == 'a')
    writeData(lockPin);
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
      client.subscribe("/doorLock");
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
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(lockPin, OUTPUT);
  digitalWrite(lockPin, LOW);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  delay(200);
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
