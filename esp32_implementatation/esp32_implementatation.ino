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
unsigned char the_topic[] = "thelamps";

unsigned long long mlen, adlen;
unsigned long long clen, mlen2;

// Variables for MQTT
const char* ssid = "E915";
const char* password =  "aileron14";
const char* mqttServer = "broker.mqtt-dashboard.com";
const int mqttPort = 1883;

const char* masterTopic = "esp/kel3/afee5de16c1badcb3dc5dfa965a";
const char* outTopic = "esp/kel3/gate";

unsigned long master_time = 0;

short currstep = 0;
String out,temp,userID;

WiFiClient espClient;
PubSubClient client(espClient);
long prevTime = 0;

void char_to_hex (char* origin, unsigned char* dest) {
  
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


void random_msg() {
  for (int i = 0; i < 8; i++) {
    msg[i] = rand() % 256;
  }
}

void setup_wifi() {
  delay(10);
  Serial.println();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  memset(cipher,'\0',25);
  memset(data,'\0',25);
  memset(data2,'\0',25); // Emptying array
  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    data[i] = (unsigned char) payload[i];
    data2[i] = (char) payload[i];
    Serial.print((char)data[i]);
  }
  Serial.println();
    
  
  if (currstep == 0) {
    if (strcmp(strtok(data2, ","),"connect") == 0) {
      strcpy((char*)ad, strtok(NULL,","));
      userID = String((const char*)ad);
      out = "Does " + userID + " may have acces to your IoT network?";
      //Serial.println(userID);
      client.publish(masterTopic, out.c_str());
      delay(10);
      
      client.subscribe(masterTopic);
      master_time = millis();
      currstep++;
    }
  }
  
  else if(currstep == 1) {
    if (String((const char*)data2) == "answer,y") {
      if (millis() - master_time < 5000) {
        
        out = "Decrypt this message to append to base topic";
        client.publish(outTopic, out.c_str());
        /*
        for (int i = 0; i < 8; i++) {
          Serial.print(the_topic[i],HEX);
        }Serial.println();
        for (int i = 0; i < 8; i++) {
          Serial.print(ad[i],HEX);
        }Serial.println();
        for (int i = 0; i < 12; i++) {
          Serial.print(nonce[i],HEX);
        }Serial.println();
        for (int i = 0; i < 16; i++) {
          Serial.print(key[i],HEX);
        }Serial.println();
        //Serial.print("\tThe length\t");
        */
        mlen = sizeof(the_topic)/sizeof(the_topic[0]) - 1;
        adlen = sizeof(ad)/sizeof(ad[0]);
        //Serial.println(adlen);

        crypto_aead_encrypt(cipher, &clen, (unsigned char*)the_topic, mlen, ad, adlen, NULL, nonce, key);
        out = hex_to_string(out, cipher, clen);
        client.publish(outTopic, out.c_str());
        for(int i = 0; i < 25; i++) {
          Serial.print(cipher[i],HEX);
        }
        Serial.println();
        Serial.println(clen);
        currstep++;
      }
      else {
        out = "Sorry you're out of time. Please retry again";
        client.publish(masterTopic,out.c_str());
        client.publish(outTopic, out.c_str());
        currstep = 0;
        reconnect();
      }
    }
    else {
      out = "Sorry you're denied entry.  Please retry again";
      client.publish(outTopic,out.c_str());
      currstep = 0;
      reconnect();
    }
    client.subscribe(outTopic);
    delay(50);
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

void setup() {
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
  client.publish(masterTopic, "Connection Made");
  mlen = sizeof(the_topic)/sizeof(the_topic[0]) - 1;
  adlen = sizeof(ad)/sizeof(ad[0]);
  
  /*
  for (int i = 0; i < 8; i++) {
    Serial.print(the_topic[i],HEX);
  }Serial.println();
  for (int i = 0; i < 8; i++) {
    Serial.print(ad[i],HEX);
  }Serial.println();
  for (int i = 0; i < 12; i++) {
    Serial.print(nonce[i],HEX);
  }Serial.println();
  for (int i = 0; i < 16; i++) {
    Serial.print(key[i],HEX);
  }Serial.println();
  //Serial.print("\tThe length\t");
  //Serial.println(adlen);
  */
  
  crypto_aead_encrypt(cipher, &clen, (unsigned char*)the_topic, mlen, ad, adlen, NULL, nonce, key);
  for(int i = 0; i < 25; i++) {
    Serial.print(cipher[i],HEX);
  }
  Serial.println();
  Serial.println(clen);
}

void loop() {
    client.loop();
}
