//
// Kapua RSA class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

#include <openssl/rand.h>
#include <openssl/rsa.h>

#include <string>

#include "Config.hpp"
#include "Logger.hpp"

namespace Kapua {

struct KeyPair {
  EVP_PKEY* publicKey;
  EVP_PKEY* privateKey;
};

struct AESContext {
  uint8_t iv[32];   // 256-bit Initialization Vector
  uint8_t key[32];  // 256-bit Session Key

  void generate() {
    RAND_bytes(key, sizeof(key));
    RAND_bytes(iv, sizeof(iv));
  }
};

class RSA {
 public:
  RSA(Logger* logger, Config* config);
  ~RSA();

  bool generate_rsa_key_pair(const std::string& publicKeyFile, const std::string& privateKeyFile, int keyBits = 2048);
  bool load_rsa_key_pair(const std::string& publicKeyFile, const std::string& privateKeyFile, KeyPair& keyPair);

  bool encrypt_aes_context(const AESContext* context, EVP_PKEY* publicKey, uint8_t* out_buffer, size_t in_size, size_t* out_size);
  bool decrypt_aes_context(AESContext* context, EVP_PKEY* privateKey, const uint8_t* in_buffer, size_t in_size, size_t* out_size);

  static size_t get_pkey_size(EVP_PKEY* pKey);

 protected:
  Logger* _logger;
  Config* _config;
};

}  // namespace Kapua
