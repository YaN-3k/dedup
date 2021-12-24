#include "sha256.h"

#include <stdio.h>

void
hash2cstr(unsigned char hash[], char cstr[])
{
    int i;

    for (i = 0; i < SHA256_LENGTH; i++)
        sprintf(cstr + (i * 2), "%02x", hash[i]);
}

int
sha256(unsigned char hash[], FILE *fp, size_t nbytes)
{
    SHA256_CTX sha_ctx;
    size_t n, total = 0;
    char buf[BUFSIZ];

    SHA256_Init(&sha_ctx);

    while ((n = fread(buf, 1, BUFSIZ, fp)) > 0 && total < nbytes) {
        if ((total += n) > nbytes) {
            n -= total - nbytes;
            total = nbytes;
        }
        SHA256_Update(&sha_ctx, buf, n);
    }

    SHA256_Final(hash, &sha_ctx);

    return total;
}
