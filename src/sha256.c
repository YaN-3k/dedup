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

int
sha256(FILE *fp, unsigned char hash[static SHA256_LENGTH])
{
    char buffer[1024];
    size_t bytes;

    SHA256_CTX sha_ctx;
    memset(buffer, 0, sizeof(buffer));
    bytes = fread(buffer, sizeof(*buffer), sizeof(buffer), fp);
    if (bytes == 0 || ferror(fp) != 0)
        return 0;

    SHA256_Init(&sha_ctx);
    SHA256_Update(&sha_ctx, buffer, sizeof(buffer));
    SHA256_Final(hash, &sha_ctx);
    return bytes;
}
