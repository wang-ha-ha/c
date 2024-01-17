#ifndef __BIGNUM_H__
#define __BIGNUM_H__

typedef  unsigned int  u32;
typedef  unsigned char u8;

#define __cache_text
#define __cache_data

typedef unsigned long long dbn_t;
typedef u32 bn_t;

#define BN_DIGIT_BITS               32      // For u32
#define BN_MAX_DIGITS               65      // RSA_MAX_MODULUS_LEN + 1

#define BN_MAX_DIGIT                0xFFFFFFFF

#define DIGIT_2MSB(x)               (u32)(((x) >> (BN_DIGIT_BITS - 2)) & 0x03)


void __cache_text bn_decode(bn_t *bn, u32 digits, u8 *hexarr, u32 size);
void __cache_text bn_encode(u8 *hexarr, u32 size, bn_t *bn, u32 digits);

void __cache_text bn_assign(bn_t *a, bn_t *b, u32 digits);                                          // a = b
void __cache_text bn_assign_zero(bn_t *a, u32 digits);                                              // a = 0

bn_t __cache_text bn_add(bn_t *a, bn_t *b, bn_t *c, u32 digits);                                    // a = b + c, return carry
bn_t __cache_text bn_sub(bn_t *a, bn_t *b, bn_t *c, u32 digits);                                    // a = b - c, return borrow
void __cache_text bn_mul(bn_t *a, bn_t *b, bn_t *c, u32 digits);                                    // a = b * c
void __cache_text bn_div(bn_t *a, bn_t *b, bn_t *c, u32 cdigits, bn_t *d, u32 ddigits);        // a = b / c, d = b % c
bn_t __cache_text bn_shift_l(bn_t *a, bn_t *b, u32 c, u32 digits);                             // a = b << c (a = b * 2^c)
bn_t __cache_text bn_shift_r(bn_t *a, bn_t *b, u32 c, u32 digits);                             // a = b >> c (a = b / 2^c)

void __cache_text bn_mod(bn_t *a, bn_t *b, u32 bdigits, bn_t *c, u32 cdigits);                 // a = b mod c
void __cache_text bn_mod_mul(bn_t *a, bn_t *b, bn_t *c, bn_t *d, u32 digits);                       // a = b * c mod d
void __cache_text bn_mod_exp(bn_t *a, bn_t *b, bn_t *c, u32 cdigits, bn_t *d, u32 ddigits);    // a = b ^ c mod d

int __cache_text bn_cmp(bn_t *a, bn_t *b, u32 digits);                                              // returns sign of a - b

u32 __cache_text bn_digits(bn_t *a, u32 digits);                                               // returns significant length of a in digits

#define BN_ASSIGN_DIGIT(a, b, digits)   {bn_assign_zero(a, digits); a[0] = b;}

void __cache_text *rsa_memset(u32 *p, u32 c, u32 nword);

void cache_init();
#endif  // __BIGNUM_H__
