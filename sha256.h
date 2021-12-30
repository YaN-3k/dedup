/* #include <openssl/sha256.h> */

#define SHA256_LENGTH      SHA256_DIGEST_LENGTH
#define SHA256_CSTR_LENGTH SHA256_DIGEST_LENGTH * 2 + 1

int sha256(unsigned char hash[], FILE *fp, size_t nbytes);
void hash2cstr(const unsigned char hash[], char cstr[]);
