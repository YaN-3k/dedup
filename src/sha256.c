#include "sha256.h"

#include <stdio.h>
#include <string.h>

#include <openssl/sha.h>

int
sha256(FILE *fp, unsigned char hash[])
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
