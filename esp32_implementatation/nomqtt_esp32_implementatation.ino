/*
#include "crypto_aead.h"
#include "api.h"

unsigned char cipher[256];
unsigned char key[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};
unsigned char nonce[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b};
unsigned char msg[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
unsigned char ad[9]= {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};

unsigned long long mlen = 8, adlen = 9;
unsigned long long clen, mlen2;

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  Serial.print("Key: ");
  for (int i = 0; i < 16; i++) {
    Serial.print(key[i], HEX);
  } Serial.println();

  Serial.print("Nonce: ");
  for (int i = 0; i < 12; i++) {
    Serial.print(nonce[i], HEX);
  } Serial.println();

  Serial.print("Message: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(msg[i], HEX);
  } Serial.println();

  Serial.print("AD: ");
  for (int i = 0; i < 9; i++) {
    Serial.print(ad[i], HEX);
  } Serial.println();

  crypto_aead_encrypt(cipher, &clen, msg, mlen, ad, adlen, NULL, nonce, key);
  Serial.print("Cipher: ");
  for (int i = 0; i < clen; i++) {
    Serial.print(cipher[i], HEX);
  } Serial.println();

  crypto_aead_decrypt(msg, &mlen2, NULL, cipher, clen, ad, adlen, nonce, key);
  Serial.print("Decipher: ");
  for (int i = 0; i < mlen2; i++) {
    Serial.print(msg[i], HEX);
  } Serial.println();
  
  
  
}

void loop() {
  // put your main code here, to run repeatedly:

}*/
