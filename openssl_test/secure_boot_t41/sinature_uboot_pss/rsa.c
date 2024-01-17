#include <rsa.h>
#include <bignum.h>
#include <string.h>

static unsigned int tmp_data[64];
static bn_t c[BN_MAX_DIGITS];

static int big_endian_word(unsigned int *out, unsigned int *input, unsigned int len)
{
	int i, j;
	for (i=len - 1, j=0; i<len; i--, j++)
		out[i] = input[j];
	return i;
}

#define BLSWAP32(val)											\
    (unsigned int)((((unsigned int)(val) & (unsigned int)0x000000ffU) << 24) |				\
		  (((unsigned int)(val) & (unsigned int)0x0000ff00U) <<	8) |			\
		  (((unsigned int)(val) & (unsigned int)0x00ff0000U) >>	8) |			\
		  (((unsigned int)(val) & (unsigned int)0xff000000U) >> 24))

void __cache_text rsa_public_decrypt(bn_t *out, bn_t *in, bn_t in_len, bn_t * n, bn_t * e, bn_t key_len)
{

	memcpy(tmp_data, in, 256);
	big_endian_word(in, tmp_data, 64);

	memcpy(tmp_data, n, 256);
	big_endian_word(n, tmp_data, 64);

	bn_mod_exp(c, in, e, 1, n, key_len);

	bn_encode((unsigned char *)out, 256, c, in_len);

	int i = 0;
	for ( i = 0; i < 64; i++ ) {
		out[i] = BLSWAP32(out[i]);
	}
}
void (*f_rsa_public_decrypt)(bn_t*, bn_t*, bn_t, bn_t*, bn_t*, bn_t) = rsa_public_decrypt;
