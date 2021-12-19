#include "sha256.h"

#include <string.h>
#include <unistd.h>

#include <openssl/sha.h>

int
sha256(int fd, int length, unsigned char hash[])
{
    char buffer[length];
    size_t bytes;

    SHA256_CTX sha_ctx;
    memset(buffer, 0, sizeof(buffer));
    bytes = read(fd, buffer, length);
    if (bytes == -1)
        return -1;
    
    SHA256_Init(&sha_ctx);
    SHA256_Update(&sha_ctx, buffer, length);
    SHA256_Final(hash, &sha_ctx);
    return bytes;
}
