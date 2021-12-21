#include "sha256.h"

#include <stdio.h>

void
hash2cstr(unsigned char hash[static SHA256_LENGTH],
          char cstr[static SHA256_CSTR_LENGTH])
{
    for (int i = 0; i < SHA256_LENGTH; i++)
        sprintf(cstr + (i * 2), "%02x", hash[i]);
}

int
sha256(unsigned char hash[static SHA256_LENGTH], const char *filepath,
       size_t nbytes)
{
    size_t n, total = 0;
    char buf[BUFSIZ];
    FILE *fp;

    if ((fp = fopen(filepath, "r")) == NULL)
        return -1;

    SHA256_CTX sha_ctx;
    SHA256_Init(&sha_ctx);

    while ((n = fread(buf, 1, BUFSIZ, fp)) > 0 && total < nbytes) {
        if ((total += n) > nbytes) {
            n -= total - nbytes;
            total = nbytes;
        }
        SHA256_Update(&sha_ctx, buf, n);
    }

    SHA256_Final(hash, &sha_ctx);
    fclose(fp);

    return total;
}
