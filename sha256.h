#ifndef SHA256_H__
#define SHA256_H__

#include <openssl/sha.h>

#define SHA256_LENGTH      SHA256_DIGEST_LENGTH
#define SHA256_CSTR_LENGTH SHA256_DIGEST_LENGTH * 2 + 1

int sha256(unsigned char hash[], const char *filepath, size_t nbytes);
void hash2cstr(unsigned char hash[], char cstr[]);

#endif