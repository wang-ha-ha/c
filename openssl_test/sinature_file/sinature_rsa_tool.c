#include <stdio.h>
#include <string.h>
#include "openssl/err.h"
#include "openssl/sha.h"
#include "openssl/des.h"    
#include "openssl/rsa.h"    
#include "openssl/pem.h"    

#include "sinature_rsa_tool.h"

#define BLOCK_UNIT  64
#define SPL_OFFSET 2*1024
#define HASH_SIZE 256

#define PAD_NUM RSA_NO_PADDING

static int getFromBignum(BIGNUM *a, uint8_t *buf, int size)
{
    char *m = BN_bn2hex(a);
    int i = 0;
    for(; i < size; i++) {
        sscanf(m+(i*2), "%02x", (unsigned int*)&buf[i]);
    }
    OPENSSL_free(m); 
    return 0;
}

static int str2Hex(char *str, int str_size, char *out, int out_len)
{
    if(str_size > out_len * 2) {
        printf("out buf short error, %s, %d\n", __func__, __LINE__);
        return -1;
    }
    int i = 0;
    char *p = str;
    for(; i < str_size; i++) {
        sscanf(p+(i*2), "%02x", (unsigned int *)&out[i]);
    }

    return 0;
}

int get_module_and_e(const char *fileName, uint8_t *module, int module_size, int *e)
{
    // RSA *rsa = NULL;
    // FILE *fp = NULL;

    // if ((fp = fopen(fileName, "r")) == NULL) {
    //     return -1;
    // }
	// rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL);
    // if (!rsa) {
    //     printf("%s, %d\n", __func__, __LINE__);
    //     return -1;
    // }
    // getFromBignum(rsa->e, (uint8_t *)e, sizeof(int));
    // getFromBignum(rsa->n, module, module_size);
    // fclose(fp);
    return 0;
}

// int pri_encrypt(const char *priname, char *buf, int size, char *out, int o_size)
// {
//     RSA* rsa ;
//     FILE *fp = NULL;

// 	unsigned char m_hash[HASH_SIZE] = {0};
// 	unsigned char em[HASH_SIZE] = {0};

// 	memcpy(m_hash, buf, size);

//     if ((fp = fopen(priname, "r")) == NULL) {
//         printf("%s, %d\n", __func__, __LINE__);
//         return -1;
//     }

//     if ((rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
//         printf("%s, %d\n", __func__, __LINE__);
//         return -1;
//     }
    
//     int len = size;
//     int rsa_len = RSA_size(rsa);
//     char *en = (char *)malloc(rsa_len + 1);
//     memset(en, 0, rsa_len + 1);
//     printf("rsa_len:%d\n", rsa_len);
// 	// char *tmp = (char *)malloc(1024);

// 	// int ret = RSA_padding_add_PKCS1_PSS(rsa, em, m_hash, EVP_sha256(), -1);

//     // printf("RSA_padding_add_PKCS1_PSS:\n");
// 	// for (int i = 0; i < 256; i++) {
// 	// 	printf("0x%02x, ", em[i]);
// 	// 	if (i % 16 == 15)
// 	// 		printf("\n");
// 	// }

//     if (RSA_private_encrypt(size, (unsigned char *)buf, (unsigned char*)en, rsa, RSA_PKCS1_PADDING) < 0) {
//         printf("RSA_private_encrypt error, %s,maybe your sha greater than mod n , %s, %d\n", ERR_error_string (ERR_get_error (), (char *) buf), __func__, __LINE__);
// 		goto ERR;
//     }

//     RSA_free(rsa);
//     memcpy(out, en, rsa_len);
//     free(en);
// 	// free(tmp);

//     return 0;
// ERR:
//     RSA_free(rsa);
//     memcpy(out, en, rsa_len);
//     free(en);
// 	// free(tmp);

//     return -1;
// }

int pri_encrypt(const char *priname, char *buf, int size, char *out, int o_size)
{
    RSA *rsa = NULL;
    FILE *fp = NULL;

	unsigned char m_hash[HASH_SIZE] = {0};
	unsigned char em[HASH_SIZE] = {0};

	memcpy(m_hash, buf, size);

    if ((fp = fopen(priname, "r")) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    if ((rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    int len = size;
    int rsa_len = RSA_size(rsa);

    char *en = (char *)malloc(rsa_len + 1);
    memset(en, 0, rsa_len + 1);

	char *tmp = (char *)malloc(1024);

	int ret = RSA_padding_add_PKCS1_PSS(rsa, em, m_hash, EVP_sha256(), -1);

	int j = 0;
	int *p = (int *)em;
	for (j = 0; j < 64; j++) {
		printf("%08x ", *(p+j));
	}
	printf("\n");

    if (RSA_private_encrypt(rsa_len, (unsigned char *)em, (unsigned char*)en, rsa, PAD_NUM) < 0) {
        printf("RSA_private_encrypt error, %s,maybe your sha greater than mod n , %s, %d\n", ERR_error_string (ERR_get_error (), (char *) buf), __func__, __LINE__);
		goto ERR;
    }

    RSA_free(rsa);
    fclose(fp);
    memcpy(out, en, rsa_len);
    free(en);
	free(tmp);

    return 0;
ERR:
    RSA_free(rsa);
    fclose(fp);
    memcpy(out, en, rsa_len);
    free(en);
	free(tmp);

    return -1;
}

int pub_decrypt(const char *priname, char *buf, int size, char *out, int o_size)
{
    RSA *rsa = NULL;
    FILE *fp = NULL;
    int ret = 0;
    if ((fp = fopen(priname, "r")) == NULL) {
        return -1;
    }

    if ((rsa = PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL)) == NULL) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    int len = size;
    int rsa_len = RSA_size(rsa);

    char *de = (char *)malloc(rsa_len + 1);
    memset(de, 0, rsa_len + 1);

    if (RSA_public_decrypt(rsa_len, (unsigned char *)buf, (unsigned char*)de, rsa, PAD_NUM) < 0) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    RSA_free(rsa);
    fclose(fp);
    /*
     *if(o_size < rsa_len) {
     *    printf("%s, %d error\n", __func__, __LINE__);
     *    return -1;
     *}
     */
    memcpy(out, de, o_size);
    return 0;
}

// int pub_decrypt(const char *priname, char *buf, int size, char *out, int o_size)
// {
//     RSA *rsa = NULL;
//     FILE *fp = NULL;
//     int ret = 0;
//     if ((fp = fopen(priname, "r")) == NULL) {
//         return -1;
//     }

//     if ((rsa = PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL)) == NULL) {
//         printf("E:%s, %d\n", __func__, __LINE__);
//         return -1;
//     }

//     int len = size;
//     int rsa_len = RSA_size(rsa);
//     printf("RSA size: %d\n", rsa_len);

//     char *de = (char *)malloc(rsa_len + 1);
//     memset(de, 0, rsa_len + 1);

//     if (RSA_public_decrypt(size, (unsigned char *)buf, (unsigned char*)de, rsa, RSA_PKCS1_PADDING) < 0) {
//         printf("E:%s, %d\n", __func__, __LINE__);
//         return -1;
//     }
//     RSA_free(rsa);
//     fclose(fp);
//     /*
//      *if(o_size < rsa_len) {
//      *    printf("%s, %d error\n", __func__, __LINE__);
//      *    return -1;
//      *}
//      */
//     memcpy(out, de, o_size);
//     return 0;
// }

int get_sha256(const char *buf, int size, uint8_t *out, int out_maxlen)
{
    if(out_maxlen < 32) {
        printf("o len error\n");
        return -1;
    }

    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, buf, size);
    SHA256_Final(out, &sha256);

    return 32;
}

int get_file_buf(const char *fileName, uint8_t *buf, long size)
{
    FILE *fp = NULL;
    if ((fp = fopen(fileName, "r")) == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long f_size = ftell(fp);
    if(size < f_size) {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    rewind(fp);

    int ret = fread(buf, 1, f_size, fp);
    fclose(fp);
    return ret;
}

int get_file_buf1(const char *fileName, uint8_t **buf)
{
    FILE *fp = NULL;
    if ((fp = fopen(fileName, "r")) == NULL) {
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long f_size = ftell(fp);
    long len = f_size;

    *buf = (uint8_t *) malloc(len + 256);
    if(*buf == NULL) {
        printf("%s, %d:buf is NULL\n", __func__, __LINE__);
        return -1;
    }

    rewind(fp);

    uint8_t * temp = *buf + 256;

    int ret = fread(temp, 1, f_size, fp);
    printf("read:%d-%ld\n", ret,len);
    fclose(fp);
    return ret + 256;
}

int get_spl_len_sd(uint8_t *buf)
{
	int *p = (int *)buf;
	int len = *(p+1) - 2 * 1024;
	return len;
}

int get_spl_len(uint8_t *buf)
{

#if 1
    int *p = (int *)buf;
    int len = *(p+3) - 2 * 1024;
    printf("len before: %d\n", len);
    if(len % BLOCK_UNIT != 0) {
        len = (len / BLOCK_UNIT) * BLOCK_UNIT + BLOCK_UNIT;
        /* 改写spl len的长度 */
        printf("change spl len\n");
        *(p+3) = len + 2 * 1024;
    }
    return len;
#else
    int *p = (int *)buf;
    int len = *(p+64+1) - 1024 *2;
    return len;
#endif
}

int save_buf_to_file(const char *fileName, uint8_t *buf, long len)
{
    FILE *sp;
    if((sp = fopen(fileName,"w+")) == NULL) {
        printf("open file error\n");
        return -1;
    }

    fwrite(buf, len, 1, sp);
    return 0;
}
