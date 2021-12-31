#include <openssl/sha.h>
#include <stdio.h>
#include "libc.h"
#include "sha256.h"

void
hash2str(const uchar hash[], char str[])
{
	int i;

	for(i = 0; i < SHA256_LEN; i++)
		sprintf(str + (i * 2), "%02x", hash[i]);
}

int
sha256(uchar hash[], FILE *fp, size_t nbytes)
{
	SHA256_CTX sha_ctx;
	char buf[BUFSIZ];
	size_t n, total;

	total = 0;
	SHA256_Init(&sha_ctx);
	while((n = fread(buf, 1, BUFSIZ, fp)) > 0 && total < nbytes) {
		if((total += n) > nbytes) {
			n -= total - nbytes;
			total = nbytes;
		}
		SHA256_Update(&sha_ctx, buf, n);
	}
	SHA256_Final(hash, &sha_ctx);
	return total;
}
