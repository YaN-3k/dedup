/* #include <openssl/sha256.h> */

#define SHA256_LEN     SHA256_DIGEST_LENGTH
#define SHA256_STR_LEN SHA256_DIGEST_LENGTH * 2 + 1

int sha256(uchar hash[], FILE *fp, size_t nbytes);
void hash2str(const uchar hash[], char str[]);
