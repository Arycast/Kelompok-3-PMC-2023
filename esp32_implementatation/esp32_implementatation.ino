/* --------------------GUIDES--------------------
Base Topic      : esp/kel3
Master Topic    : esp/kel3/afee5de16c1badcb3dc5dfa965a
Public Topic    : esp/kel3/gate
Spesific Topic  : esp/kel3/<category of device>
    category of device (eg/ lamp, door, heater, ac, etx) is encrypted
    Plain Text and AD  is the category of Device itself
    
Process:
- New user go to public topic. if want to get acces to iot, send message to mqtt with format
  connect,userId,cateogry
  userId must consist of 8 ASCII character that will be used as assosicated data
- User will be given encrypted message. Said message has to be devrypted and send trhough mqtt with format
  decrypt,<decryption result in HEX>

- If decryption is correct, Ask question about the user
- The answer is in format
  answer,<the answer>
- if answer matched, the user will be given an encrypted spesific topic that has to be decrypted and concatenated to the base topic

*/
#include "crypto_aead.h"
#include "api.h"
#include <WiFi.h>
#include <PubSubClient.h>


// Variables for GRAIN
unsigned char cipher[25];
unsigned char key[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f};
unsigned char nonce[12] = {0x6b,0x65,0x6c,0x33,0x70,0x70,0x6d,0x63,0x32,0x30,0x32,0x31};
unsigned char msg[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
unsigned char ad[8]= {'0','1','2','3','4','5','6','7'};
unsigned char new_data[8];
unsigned char data[25];
char data2[25];
const char topic[] = "thelamps";

unsigned long long mlen, adlen;
unsigned long long clen, mlen2;

// Variables for MQTT
const char* ssid = "K4I";
const char* password =  "senirokalil";
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;
const char* outTopic = "esp/kel3";

short currstep = 0;
String out,temp;

WiFiClient espClient;
PubSubClient client(espClient);
long prevTime = 0;

String char_to_string(String outString, unsigned char* inChar, int lenArr) {
  outString = "";
  for (int i = 0; i < lenArr; i++) {
    outString += String(inChar[i]);
  }
  return outString;
}

String hex_to_string(String outString, unsigned char* inHex, int lenArr) {
  outString = "";
  for (int i = 0; i < lenArr; i++) {
    if (inHex[i] < 0x10) {
      outString += "0";
    }
    outString += String(inHex[i],HEX);
  }
  return outString;
}

void encrypt() {
  mlen = sizeof(msg)/sizeof(msg[0]);
  adlen = sizeof(ad)/sizeof(ad[0]);
  crypto_aead_encrypt(cipher, &clen, msg, mlen, ad, adlen, NULL, nonce, key);
}

void random_msg() {
  for (int i = 0; i < 8; i++) {
    msg[i] = rand() % 256;
  }
}

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
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    data[i] = (unsigned char) payload[i];
    data2[i] = (char) payload[i];
    Serial.print((char)data[i]);
  }
  Serial.println();
  
  // Serial.println(strtok((char*)data,","));     THIS WORKK!!

  if (strcmp(strtok((char*)data, ","),"connect") == 0) {
    // Copy userID as AD for encrypt
    strcpy((char*)ad,strtok(NULL,","));
    out = "Decrypt this!";
    client.publish(outTopic, out.c_str());
    delay(10);
    encrypt();
    out = hex_to_string(out,cipher,clen);
    client.publish(outTopic, out.c_str());
    currstep++;
  }
  else if (currstep == 1) {
    //Serial.println(strcmp(strtok(NULL,","),(char*)msg) == 0);
    temp = "decrypt," + char_to_string(out,msg,8);
    out = "";
    for (int i = 0; i < length; i++) {
      out += String(data2[i]);
    }
    
    if (strcmp(out.c_str(),temp.c_str()) == 0){
      currstep++;
    }
  }
  else if (currstep == 2) {
    out = "";
    for (int i = 0; i < length; i++) {
      out += String(data2[i]);
    }
    if (strcmp(out.c_str(),"answer,grain") == 0){
      currstep++;
      out = "CORRECT!!";
      client.publish(outTopic, out.c_str());
      out = "Decrypt this message to append to base topic";
      client.publish(outTopic, out.c_str()); 
    }
  }
  else if (currstep == 3) {
    mlen = sizeof(topic)/sizeof(topic[0]) - 1;
    adlen = sizeof(ad)/sizeof(ad[0]);
    crypto_aead_encrypt(cipher, &clen, (unsigned char*)topic, mlen, ad, adlen, NULL, nonce, key);
    out = hex_to_string(out, cipher, clen);
    client.publish(outTopic, out.c_str());
    currstep++;
  }
  
  Serial.print("Success: ");
  Serial.print(currstep);
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

void decrypt() {
  //crypto_aead_decrypt(msg_dec, &mlen2, NULL, cipher, clen, ad, adlen, nonce, key);
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
}

void loop() {
    client.loop();
}
