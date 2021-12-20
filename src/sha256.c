#include "sha256.h"

#include <stdio.h>
#include <string.h>

#include <openssl/sha.h>

void
hash2cstr(unsigned char hash[static SHA256_LENGTH],
          char cstr[static SHA256_CSTR_LENGTH])
{
    int n = 0;

    for (int i = 0; i < SHA256_LENGTH; i++)
        n += sprintf(cstr + n, "%x", hash[i]);
}

void
sha256(unsigned char hash[static SHA256_LENGTH], const void *data, size_t len)
{
    SHA256_CTX sha_ctx;
    SHA256_Init(&sha_ctx);
    SHA256_Update(&sha_ctx, data, len);
    SHA256_Final(hash, &sha_ctx);
}
