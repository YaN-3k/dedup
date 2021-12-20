#ifndef SHA256_H__
#define SHA256_H__

#include <stdio.h>
#include <openssl/sha.h>

#define SHA256_LENGTH      SHA256_DIGEST_LENGTH
#define SHA256_CSTR_LENGTH SHA256_DIGEST_LENGTH * 2 + 1

void sha256(unsigned char hash[static SHA256_LENGTH],
           const void *data, size_t len);
void hash2cstr(unsigned char hash[static SHA256_LENGTH],
               char cstr[static SHA256_CSTR_LENGTH]);

#endif
