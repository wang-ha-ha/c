#ifndef _OPENSSL_TOOL__H_
#define _OPENSSL_TOOL__H_

extern void md5(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr);
extern void sha256(const std::string &srcStr, std::string &encodedStr, std::string &encodedHexStr);
extern std::string des_encrypt(const std::string &clearText, const std::string &key);
extern std::string des_decrypt(const std::string &cipherText, const std::string &key);
extern void generateRSAKey(std::string strKey[2]);
extern std::string rsa_pub_encrypt(const std::string &clearText, const std::string &pubKey);
extern std::string rsa_pri_decrypt(const std::string &cipherText, const std::string &priKey);
extern std::string rsa_pri_encrypt(const std::string &clearText,  std::string &pubKey);
extern std::string rsa_pub_decrypt(const std::string &cipherText,  std::string &pubKey);
extern std::string rsa_pri_split117_encrypt(const std::string &clearText,  std::string &pubKey);
extern std::string rsa_pub_split128_decrypt(const std::string &clearText,  std::string &pubKey);

#endif /* _OPENSSL_TOOL__H_ */