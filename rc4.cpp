#include "rc4.h"

unsigned char S[256];
unsigned int i, j;

void rc4_init(unsigned const char* key, unsigned int key_length)
{
	unsigned char temp;

	for (i = 0; i != 256; ++i)
		S[i] = i;

	for (i = j = 0; i != 256; ++i)
	{
		j = (j + key[i % key_length] + S[i]) % 256;
		temp = S[i];
		S[i] = S[j];
		S[j] = temp;
	}

	i = j = 0;
}

unsigned char rc4_output()
{
	unsigned char temp;

	i = (i + 1) % 256;
	j = (j + S[i]) % 256;

	temp = S[j];
	S[j] = S[i];
	S[i] = temp;

	return S[(temp + S[j]) % 256];
}