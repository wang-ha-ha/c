#include <iostream>    
#include <cassert>  
#include <string>    
#include <vector>    
#include "openssl/md5.h"    
#include "openssl/sha.h"    
#include "openssl/des.h"    
#include "openssl/rsa.h"    
#include "openssl/pem.h"    

#include <stdio.h>
#include <string.h>
#include "base64.h"
#include "openssl_tool.h"

// ---- md5摘要哈希 ---- //    
void md5(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr)  
{  
    // 调用md5哈希    
    unsigned char mdStr[33] = {0};  
    MD5((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);  
  
    // 哈希后的字符串    
    encodedStr = std::string((const char *)mdStr);  
    encodedHexStr = base64_encode(encodedStr);
    // // 哈希后的十六进制串 32字节    
    // char buf[65] = {0};  
    // char tmp[3] = {0};  
    // for (int i = 0; i < 32; i++)  
    // {  
    //     sprintf(tmp, "%02x", mdStr[i]);  
    //     strcat(buf, tmp);  
    // }  
    // buf[32] = '\0'; // 后面都是0，从32字节截断    
    // encodedHexStr = std::string(buf);  
}  
  
// ---- sha256摘要哈希 ---- //    
void sha256(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr)  
{  
    // 调用sha256哈希    
    unsigned char mdStr[33] = {0};  
    SHA256((const unsigned char *)srcStr.c_str(), srcStr.length(), mdStr);  
  
    // 哈希后的字符串    
    encodedStr = std::string((const char *)mdStr);  
    // 哈希后的十六进制串 32字节    
    char buf[65] = {0};  
    char tmp[3] = {0};  
    for (int i = 0; i < 32; i++)  
    {  
        sprintf(tmp, "%02x", mdStr[i]);  
        strcat(buf, tmp);  
    }  
    buf[32] = '\0'; // 后面都是0，从32字节截断    
    encodedHexStr = std::string(buf);  
}  
  
// ---- des对称加解密 ---- //    
// 加密 ecb模式    
std::string des_encrypt(const std::string &clearText, const std::string &key)  
{  
    std::string cipherText; // 密文    
  
    DES_cblock keyEncrypt;  
    memset(keyEncrypt, 0, 8);  
  
    // 构造补齐后的密钥    
    if (key.length() <= 8)  
        memcpy(keyEncrypt, key.c_str(), key.length());  
    else  
        memcpy(keyEncrypt, key.c_str(), 8);  
  
    // 密钥置换    
    DES_key_schedule keySchedule;  
    DES_set_key_unchecked(&keyEncrypt, &keySchedule);  
  
    // 循环加密，每8字节一次    
    const_DES_cblock inputText;  
    DES_cblock outputText;  
    std::vector<unsigned char> vecCiphertext;  
    unsigned char tmp[8];  
  
    for (int i = 0; i < clearText.length() / 8; i++)  
    {  
        memcpy(inputText, clearText.c_str() + i * 8, 8);  
        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);  
        memcpy(tmp, outputText, 8);  
  
        for (int j = 0; j < 8; j++)  
            vecCiphertext.push_back(tmp[j]);  
    }  
  
    if (clearText.length() % 8 != 0)  
    {  
        int tmp1 = clearText.length() / 8 * 8;  
        int tmp2 = clearText.length() - tmp1;  
        memset(inputText, 0, 8);  
        memcpy(inputText, clearText.c_str() + tmp1, tmp2);  
        // 加密函数    
        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_ENCRYPT);  
        memcpy(tmp, outputText, 8);  
  
        for (int j = 0; j < 8; j++)  
            vecCiphertext.push_back(tmp[j]);  
    }  
  
    cipherText.clear();  
    cipherText.assign(vecCiphertext.begin(), vecCiphertext.end());  
  
    return cipherText;  
}  
  
// 解密 ecb模式    
std::string des_decrypt(const std::string &cipherText, const std::string &key)  
{  
    std::string clearText; // 明文    
  
    DES_cblock keyEncrypt;  
    memset(keyEncrypt, 0, 8);  
  
    if (key.length() <= 8)  
        memcpy(keyEncrypt, key.c_str(), key.length());  
    else  
        memcpy(keyEncrypt, key.c_str(), 8);  
  
    DES_key_schedule keySchedule;  
    DES_set_key_unchecked(&keyEncrypt, &keySchedule);  
  
    const_DES_cblock inputText;  
    DES_cblock outputText;  
    std::vector<unsigned char> vecCleartext;  
    unsigned char tmp[8];  
  
    for (int i = 0; i < cipherText.length() / 8; i++)  
    {  
        memcpy(inputText, cipherText.c_str() + i * 8, 8);  
        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_DECRYPT);  
        memcpy(tmp, outputText, 8);  
  
        for (int j = 0; j < 8; j++)  
            vecCleartext.push_back(tmp[j]);  
    }  
  
    if (cipherText.length() % 8 != 0)  
    {  
        int tmp1 = cipherText.length() / 8 * 8;  
        int tmp2 = cipherText.length() - tmp1;  
        memset(inputText, 0, 8);  
        memcpy(inputText, cipherText.c_str() + tmp1, tmp2);  
        // 解密函数    
        DES_ecb_encrypt(&inputText, &outputText, &keySchedule, DES_DECRYPT);  
        memcpy(tmp, outputText, 8);  
  
        for (int j = 0; j < 8; j++)  
            vecCleartext.push_back(tmp[j]);  
    }  
  
    clearText.clear();  
    clearText.assign(vecCleartext.begin(), vecCleartext.end());  
  
    return clearText;  
}  
  
  
// ---- rsa非对称加解密 ---- //    
#define KEY_LENGTH  1024             // 密钥长度  
#define PUB_KEY_FILE "pubkey.pem"    // 公钥路径  
#define PRI_KEY_FILE "prikey.pem"    // 私钥路径  
  
// 函数方法生成密钥对   
void generateRSAKey(std::string strKey[2])
{  
    // 公私密钥对    
    size_t pri_len;  
    size_t pub_len;  
    char *pri_key = NULL;  
    char *pub_key = NULL;  
  
    // 生成密钥对    
    RSA *keypair = RSA_generate_key(KEY_LENGTH, RSA_3, NULL, NULL);  
  
    BIO *pri = BIO_new(BIO_s_mem());  
    BIO *pub = BIO_new(BIO_s_mem());  
  
    PEM_write_bio_RSAPrivateKey(pri, keypair, NULL, NULL, 0, NULL, NULL);  
    PEM_write_bio_RSAPublicKey(pub, keypair);  
  
    // 获取长度    
    pri_len = BIO_pending(pri);  
    pub_len = BIO_pending(pub);  
  
    // 密钥对读取到字符串    
    pri_key = (char *)malloc(pri_len + 1);  
    pub_key = (char *)malloc(pub_len + 1);  
  
    BIO_read(pri, pri_key, pri_len);  
    BIO_read(pub, pub_key, pub_len);  
  
    pri_key[pri_len] = '\0';  
    pub_key[pub_len] = '\0';  
  
    // 存储密钥对    
    strKey[0] = pub_key;  
    strKey[1] = pri_key;  
  
    // 存储到磁盘（这种方式存储的是begin rsa public key/ begin rsa private key开头的）  
    FILE *pubFile = fopen(PUB_KEY_FILE, "w");  
    if (pubFile == NULL)  
    {  
        assert(false);  
        return;  
    }  
    fputs(pub_key, pubFile);  
    fclose(pubFile);  
  
    FILE *priFile = fopen(PRI_KEY_FILE, "w");  
    if (priFile == NULL)  
    {  
        assert(false);  
        return;  
    }  
    fputs(pri_key, priFile);  
    fclose(priFile);  
  
    // 内存释放  
    RSA_free(keypair);  
    BIO_free_all(pub);  
    BIO_free_all(pri);  
  
    free(pri_key);  
    free(pub_key);  
}  
  
// 命令行方法生成公私钥对（begin public key/ begin private key）  
// 找到openssl命令行工具，运行以下  
// openssl genrsa -out prikey.pem 1024   
// openssl rsa - in privkey.pem - pubout - out pubkey.pem  
  
// 公钥加密    
std::string rsa_pub_encrypt(const std::string &clearText, const std::string &pubKey)  
{  
    std::string strRet;  
    RSA *rsa = NULL;  
    BIO *keybio = BIO_new_mem_buf((unsigned char *)pubKey.c_str(), -1);  
    // 此处有三种方法  
    // 1, 读取内存里生成的密钥对，再从内存生成rsa  
    // 2, 读取磁盘里生成的密钥对文本文件，在从内存生成rsa  
    // 3，直接从读取文件指针生成rsa  
    RSA* pRSAPublicKey = RSA_new();
    std::string pkcs1_header = "-----BEGIN RSA PUBLIC KEY-----";
	std::string pkcs8_header = "-----BEGIN PUBLIC KEY-----";
    if( 0 == strncmp(pubKey.c_str(),pkcs8_header.c_str(),pkcs8_header.length()))
    {
        std::cout << "pkcs8_header" << std::endl;
		rsa = PEM_read_bio_RSA_PUBKEY(keybio,&rsa,NULL,NULL);
	}
	else if(0 == strncmp(pubKey.c_str(),pkcs1_header.c_str(),pkcs1_header.length()))
    {
        std::cout << "pkcs1_header" << std::endl;
		rsa = PEM_read_bio_RSAPublicKey(keybio,&rsa,NULL,NULL);
	}
    else
    {
        std::cout << "asdasd " << std::endl;
    }
    // rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);  
  
    int len = RSA_size(rsa);  
    char *encryptedText = (char *)malloc(len + 1);  
    memset(encryptedText, 0, len + 1);  
  
    // 加密函数  
    int ret = RSA_public_encrypt(clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);  
    if (ret >= 0)  
    {
        strRet = std::string(encryptedText, ret);
        strRet = base64_encode(strRet,ret); 
    }

    // 释放内存  
    free(encryptedText);  
    BIO_free_all(keybio);  
    RSA_free(rsa);  
  
    return  strRet;  
}  
  
// 私钥解密    
std::string rsa_pri_decrypt(const std::string &cipherText, const std::string &priKey)  
{  
    std::string strRet;  
    RSA *rsa = RSA_new();  
    BIO *keybio;  
    keybio = BIO_new_mem_buf((unsigned char *)priKey.c_str(), -1);  
  
    // 此处有三种方法  
    // 1, 读取内存里生成的密钥对，再从内存生成rsa  
    // 2, 读取磁盘里生成的密钥对文本文件，在从内存生成rsa  
    // 3，直接从读取文件指针生成rsa  
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);  
  
    int len = RSA_size(rsa);  
    char *decryptedText = (char *)malloc(len + 1);  
    memset(decryptedText, 0, len + 1);  
    
    std::string c = base64_decode(cipherText);
    // 解密函数  
    int ret = RSA_private_decrypt(c.length(), (const unsigned char*)c.c_str(), (unsigned char*)decryptedText, rsa, RSA_PKCS1_PADDING);  
    if (ret >= 0)  
        strRet = std::string(decryptedText, ret);  
  
    // 释放内存  
    free(decryptedText);  
    BIO_free_all(keybio);  
    RSA_free(rsa);  
  
    return strRet;  
}  

// 私钥加密
std::string rsa_pri_encrypt(const std::string &clearText,  std::string &pubKey)  
{  
    std::string strRet;  
    BIO *keybio = BIO_new_mem_buf((unsigned char *)pubKey.c_str(), -1);  
    // 此处有三种方法  
    // 1, 读取内存里生成的密钥对，再从内存生成rsa  
    // 2, 读取磁盘里生成的密钥对文本文件，在从内存生成rsa  
    // 3，直接从读取文件指针生成rsa  
    //RSA* pRSAPublicKey = RSA_new();  
    RSA* rsa = RSA_new();
    rsa = PEM_read_bio_RSAPrivateKey(keybio, &rsa, NULL, NULL);
    if (!rsa)
    {
        BIO_free_all(keybio);
        return std::string("");
    }

    int len = RSA_size(rsa);  
    //int len = 1028;
    char *encryptedText = (char *)malloc(len + 1);  
    memset(encryptedText, 0, len + 1);  
  
    // 加密  
    int ret = RSA_private_encrypt(clearText.length(), (const unsigned char*)clearText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);  
    if (ret >= 0)  
    {
        strRet = std::string(encryptedText, ret);
        // strRet = base64_encode(strRet,ret); 
    }
  
    // 释放内存  
    free(encryptedText);  
    BIO_free_all(keybio);  
    RSA_free(rsa);  
  
    return strRet;  
}

// 公钥解密    
std::string rsa_pub_decrypt(const std::string &cipherText,  std::string &pubKey)  
{  
    std::string strRet;  
    BIO *keybio = BIO_new_mem_buf((unsigned char *)pubKey.c_str(), -1);  
    //keybio = BIO_new_mem_buf((unsigned char *)strPublicKey.c_str(), -1);  
    // 此处有三种方法  
    // 1, 读取内存里生成的密钥对，再从内存生成rsa  
    // 2, 读取磁盘里生成的密钥对文本文件，在从内存生成rsa  
    // 3，直接从读取文件指针生成rsa  
    //RSA* pRSAPublicKey = RSA_new();  
    RSA* rsa = RSA_new();

    std::string pkcs1_header = "-----BEGIN RSA PUBLIC KEY-----";
	std::string pkcs8_header = "-----BEGIN PUBLIC KEY-----";
    if( 0 == strncmp(pubKey.c_str(),pkcs8_header.c_str(),pkcs8_header.length())){
		rsa = PEM_read_bio_RSA_PUBKEY(keybio,&rsa,NULL,NULL);
	}
	else if(0 == strncmp(pubKey.c_str(),pkcs1_header.c_str(),pkcs1_header.length())){
		rsa = PEM_read_bio_RSAPublicKey(keybio,&rsa,NULL,NULL);
	}

    // rsa = PEM_read_bio_RSAPublicKey(keybio, &rsa, NULL, NULL);
    if (!rsa)
    {
        BIO_free_all(keybio);
        return std::string("");
    }

    int len = RSA_size(rsa);  
    //int len = 1028;
    char *encryptedText = (char *)malloc(len + 1);  
    memset(encryptedText, 0, len + 1);  
    
    // std::string c = base64_decode(cipherText);
    //解密
    int ret = RSA_public_decrypt(cipherText.length(), (const unsigned char*)cipherText.c_str(), (unsigned char*)encryptedText, rsa, RSA_PKCS1_PADDING);  
    if (ret >= 0)  
        strRet = std::string(encryptedText, ret);  
  
    // 释放内存  
    free(encryptedText);  
    BIO_free_all(keybio);  
    RSA_free(rsa);  
  
    return strRet;  
}

//私钥加密 + 分片
std::string rsa_pri_split117_encrypt(const std::string &clearText,  std::string &pubKey)
{
    std::string result;
    std::string input;
    result.clear();
    std::cout << clearText.length() << std::endl;
    for(int i = 0 ; i < clearText.length()/117; i++)
    {
           input.clear();
           input.assign(clearText.begin() + i*117, clearText.begin() + i*117 + 117);
           result = result + rsa_pri_encrypt(input, pubKey);
    }
    if( clearText.length()%117 != 0)
    {
        int tem1 = clearText.length()/117*117;
        int tem2 = clearText.length() - tem1;
        input.clear();
        input.assign(clearText.begin()+ tem1, clearText.end());
        result = result + rsa_pri_encrypt(input, pubKey);
    }
    return base64_encode(result);
}

//公钥解密 + 分片
std::string rsa_pub_split128_decrypt(const std::string &clearText,  std::string &pubKey)
{
    //Base64 *base = new Base64();
    std::string result;
    std::string input;
    std::string c = base64_decode(clearText);
    result.clear();
    std::cout << c.length() << std::endl;
    for(int i = 0 ; i< c.length()/128; i++)
    {
        input.clear();
        input.assign(c.begin() + i*128, c.begin() + i*128 + 128);

        result = result + rsa_pub_decrypt(input, pubKey);
    }
    if(c.length()%128 != 0)
    {
        int tem1 = c.length()/128 * 128;
        int tem2 = c.length() - tem1;
        input.clear();
        input.assign(c.begin()+ tem1, c.end());
        result = result + rsa_pri_encrypt(input, pubKey);
    }
    return result;
}

#ifdef OPENSSL_DEMO_TEST
int main(int argc, char **argv)  
{  
    // 原始明文    
    std::string srcText = "this is an example";  
  
    std::string encryptText;  
    std::string encryptHexText;  
    std::string decryptText;  
  
    std::cout << "=== 原始明文 ===" << std::endl;  
    std::cout << srcText << std::endl;  
    std::string encodedHexStr = base64_encode(srcText);
    std::cout << "base64： " << encodedHexStr << std::endl;  
    // // md5    
    // std::cout << "=== md5哈希 ===" << std::endl;  
    // md5(srcText, encryptText, encryptHexText);  
    // std::cout << "摘要字符： " << encryptText << std::endl;  
    // std::cout << "摘要串： " << encryptHexText << std::endl;  
  
    // // sha256    
    // std::cout << "=== sha256哈希 ===" << std::endl;  
    // sha256(srcText, encryptText, encryptHexText);  
    // std::cout << "摘要字符： " << encryptText << std::endl;  
    // std::cout << "摘要串： " << encryptHexText << std::endl;  
  
    // // des    
    // std::cout << "=== des加解密 ===" << std::endl;  
    // std::string desKey = "12345";  
    // encryptText = des_encrypt(srcText, desKey);  
    // std::cout << "加密字符： " << std::endl;  
    // std::cout << encryptText << std::endl;  
    // decryptText = des_decrypt(encryptText, desKey);  
    // std::cout << "解密字符： " << std::endl;  
    // std::cout << decryptText << std::endl;  
  
    // rsa    
    std::cout << "=== rsa加解密 ===" << std::endl;  
    std::string key[2];  
    generateRSAKey(key);  
    // std::cout << "公钥: " << std::endl;  
    // std::cout << key[0] << std::endl;  
    // std::cout << "私钥： " << std::endl;  
    // std::cout << key[1] << std::endl;
//     key[0]="-----BEGIN RSA PUBLIC KEY-----\n\
// MIIBCAKCAQEAwEc0ms+HSBph5yiOPMW3MKPR0vFiimNoaITHUR75SWBwaUmvQMT9\n\
// oURgLX+8Gx7nVygdAEdyxb4Ze4/2oXgYCo5opSmbHvTTdSXaoEpQheY+VJJNjC9p\n\
// MdZ4STPfujsAqVsBB2WeVXkDmkeVcH87285uArk0SQjeWzT2HoVKTmLLZ16PsEDh\n\
// 6p7SxMmkKhM9yTwtJMVUPlxPHAkHkajT1qTlVsq0OMG+J3sN8LtFN/nf3W9ngKam\n\
// JwT8+i4d3MYDxT4VxZg1AvNSAhJsskpY7HrNxW+lSQqvwH9X+l/SFQzEU0Eatk9g\n\
// qbYU2SpjZYxTMgKY+t2FeGAfh1EKZJRtcwIBAw==\n\
// -----END RSA PUBLIC KEY-----\n\
// ";

    encryptText = rsa_pub_encrypt(srcText, key[0]);
    std::cout << "加密字符： " << std::endl;  
    std::cout << encryptText << std::endl;  
    // decryptText = rsa_pri_decrypt(encryptText, key[1]);  
    // std::cout << "解密字符： " << std::endl;  
    // std::cout << decryptText << std::endl;  

    encryptText = rsa_pri_encrypt(srcText, key[1]);
    std::cout << "加密字符： " << std::endl;  
    std::cout << encryptText << std::endl;  
    decryptText = rsa_pub_decrypt(encryptText, key[0]);  
    std::cout << "解密字符： " << std::endl;  
    std::cout << decryptText << std::endl;  
    
    encryptText = rsa_pri_split117_encrypt(srcText+"aasdasdasdasdasdaasdasdasdasdasfgggsdvasdasdasdasdasfgggasaasdasdasdasdasdaasdasdasdasdasfgggsdvasdasdasdasdasfgggasfgggasdasdasfgggsasdasdasdasdasfgggdasdasdasdasdasdasfgggasdasdasdasdasfgggasdasdasdaasdasdasdasdasdaasdasdasdasdasfgggsdvasdasdasdasdasfgggasfgggasdasdasfgggsasdasdasdasdasfgggdasdasdasdasdasdasfgggasdasdasdasdasfgggasdasdasdfgggasdasdasfgggsasdasdasdasdasfgggdasdasdasdasdasdasfgggasdasdasdasdasfgggasdasdasdasdasfgggasdasdasfggg", key[1]);
    std::cout << "加密字符： " << std::endl;  
    std::cout << encryptText << std::endl;  
    decryptText = rsa_pub_split128_decrypt(encryptText, key[0]);  
    std::cout << "解密字符： " << std::endl;  
    std::cout << decryptText << std::endl;  
    
    key[0]="-----BEGIN PUBLIC KEY-----\n\
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCOpYZ0vc/0U1hM3F91txZYhQeA\n\
6gh0sfcuzSWFMr610BB9NO7WxrbspMfyuMyjqZnlLIAVXKvee/AYSr4GHy/EEnQB\n\
LTXPZ3r7B8Jc19ZbEs307ouyXQ7Dj3K3x5xjLzHfQuJkxywzP5CdBXpHPHs12o5X\n\
NgpQl1JAyzOqUzpqKQIDAQAB\n\
-----END PUBLIC KEY-----\n";
    
    encryptText = "c2TNJuxYmxjZpDN4AmBaCm/5Lh0WV5+hrh0f9o/DM6I2/DkdldzUVmiAU6ZIkWlQX5J2KAzP12mwJ/sHvJOR3cIc8CET9FMixyi+81dNF+MLyZ8eNn9KbsyGzfvrtcK/aX9yZGwcvq1NL0cQXuhpPrMeCS/F44Inw8CXZFK/zOAsXJTG/XKYqPNVmDrPRVpTtg5Dbm4DnMBXoR4lfWzgYJG2eZ8jbKCoPSX0e9rKoTaj0HtB15/saMGriTIIC3TKFlaw8HKiRx1vWAn13XfbATlqrAA9E43senDlquSaUps6zzn5RXwTj/FRrVDZI4hkWCfqKtrFC1ojSDw9N/EmzA==";
    decryptText = rsa_pub_split128_decrypt(encryptText, key[0]);  
    std::cout << "解密字符： " << std::endl;  
    std::cout << decryptText << std::endl;  
    return 0;  
}
#endif