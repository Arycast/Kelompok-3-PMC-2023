#include "crypto_aead.h"
#include "api.h"
#include <WiFi.h>
#include <PubSubClient.h>


// Variables for GRAIN
unsigned char cipher[256];
unsigned char cipher2[256];
unsigned char key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
unsigned char nonce[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
unsigned char msg[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
unsigned char ad[9]= {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
unsigned char msg_dec[8];

unsigned long long mlen = 8, adlen = 9;
unsigned long long clen, mlen2;

// Variables for MQTT
const char* ssid = "plafon 11A";
const char* password =  "11a_wifi";
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;

const char* outTopic = "esp/kel3";

long lastMsg = 0;
char out_msg[75] = {'\0'};
char buff[50] = {0};
int value = 0;

WiFiClient espClient;
PubSubClient client(espClient);
long prevTime = 0;

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
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    cipher2[i] = payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID for ESP32
    char* clientId = "kel3-ESP32";
    
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");
      // Subscribe
      client.subscribe(outTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void encrypt() {
  crypto_aead_encrypt(cipher, &clen, msg, mlen, ad, adlen, NULL, nonce, key);
}

void decrypt() {
  crypto_aead_decrypt(msg_dec, &mlen2, NULL, cipher, clen, ad, adlen, nonce, key);
}

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
  client.publish(outTopic, "Connection Made");
/*
  char *temp = buff;
  for (int i = 0; i < 16; i++) {
    sprintf(temp, "%x", key[i]);
    temp++;
  }
  snprintf(out_msg, 75, "Key: %s", buff);
  client.publish(outTopic, out_msg);
  //Serial.println(out_msg);

  memset(buff, 0, 50);
  temp = &buff[0];
  for (int i = 0; i < 12; i++) {
    sprintf(temp, "%x", nonce[i]);
    temp++;
  }
  snprintf(out_msg, 75, "Nonce: %s", buff);
  client.publish(outTopic, out_msg);
  
  memset(buff, 0, 50);
  temp = &buff[0];
  for (int i = 0; i < 8; i++) {
    sprintf(temp, "%x", msg[i]);
    temp++;
  }
  snprintf(out_msg, 75, "Message: %s", buff);
  client.publish(outTopic, out_msg);

  memset(buff, 0, 50);
  temp = &buff[0];
  for (int i = 0; i < 9; i++) {
    sprintf(temp, "%x", ad[i]);
    temp++;
  }
  snprintf(out_msg, 75, "AD: %s", buff);
  client.publish(outTopic, out_msg);*/

  encrypt();
  String keyStr = "";
  for (int i = 0; i < clen; i++) {
    if (cipher[i] < 0x10) {
      keyStr += "0";
    }
    keyStr += String(cipher[i], HEX);
  }
  String out = "Ciphered: " + keyStr;
  client.publish(outTopic, out.c_str());
  Serial.println(keyStr);

  decrypt();
  keyStr = ""; out = "";
  for (int i = 0; i < mlen2; i++) {
    if (cipher[i] < 0x10) {
      keyStr += "0";
    }
    keyStr += String(msg_dec[i], HEX);
  }
  out = "Deciphered: " + keyStr;
  client.publish(outTopic, out.c_str());
  Serial.println(keyStr);
}

  


void loop() {
    client.loop();
}
