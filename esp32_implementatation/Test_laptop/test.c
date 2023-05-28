#include "grain128aead-v2_opt.h"
#include <stdio.h>
#include <time.h>
#include <memory.h>
int crypto_aead_encrypt(
	unsigned char *c, unsigned long long *clen,
	const unsigned char *m, unsigned long long mlen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *nsec,
	const unsigned char *npub,
	const unsigned char *k)
{
	return grain_aead_encdec<0>(c, clen, m, mlen, ad, adlen, npub, k);
}
int crypto_aead_decrypt(
	unsigned char *m, unsigned long long *mlen,
	unsigned char *nsec,
	const unsigned char *c, unsigned long long clen,
	const unsigned char *ad, unsigned long long adlen,
	const unsigned char *npub,
	const unsigned char *k
)
{
	return grain_aead_encdec<1>(m, mlen, c, clen, ad, adlen, npub, k);
}
int main(){
	clock_t t;
	t = clock();
	unsigned char key[16] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f};
	unsigned char nonce[12] = {0x6b,0x65,0x6c,0x33,0x70,0x70,0x6d,0x63,0x32,0x30,0x32,0x31};
	unsigned char msg[] = "answer,y";
	unsigned char ad[]= {'0','1','2','3','4','5','6','7'};
	unsigned char cipher[256];

	unsigned long long mlen = sizeof(msg)/sizeof(msg[0]) - 1, adlen = sizeof(ad)/sizeof(ad[0]);
	unsigned long long clen, mlen2;

	unsigned char decipher[mlen];

	crypto_aead_encrypt(cipher, &clen, msg, mlen, ad, adlen, NULL, nonce, key);
	//printf("%d %d\n", mlen, adlen);
	// for (int i = 0; i < adlen; i++) {
	// 	printf("%d", ad[i]);
	// }
	printf("\n");
	for (int i = 0; i < clen; i++) {
		printf("%x", cipher[i]);
	}

	crypto_aead_decrypt(decipher, &mlen2, NULL, cipher, clen, ad, adlen, nonce, key);
	printf("\n");
	for (int i = 0; i < mlen2; i++) {
		printf("%c", decipher[i]);
	}

	printf("\n%d\n", clen);
	t = clock()-t;
	printf("clock : %d, selang : %f",t, ((float)t)/CLOCKS_PER_SEC);

}
