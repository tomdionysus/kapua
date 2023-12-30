//
// Kapua RSA class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#pragma once

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
  uint8_t iv[32];           // 256-bit Initialization Vector
  uint8_t session_key[32];  // 256-bit Session Key
};

class RSA {
 public:
  RSA(Logger* logger, Config* config);
  ~RSA();

  bool generateRSAKeyPair(const std::string& publicKeyFile, const std::string& privateKeyFile, int keyBits = 4096);
  bool loadRSAKeyPair(const std::string& publicKeyFile, const std::string& privateKeyFile, KeyPair& keyPair);

  bool encryptContext(const AESContext* context, EVP_PKEY* publicKey, uint8_t* out_buffer, size_t in_size, size_t* out_size);
  bool decryptContext(AESContext* context, EVP_PKEY* privateKey, const uint8_t* in_buffer, size_t in_size, size_t* out_size);

 protected:
  Logger* _logger;
  Config* _config;
};

}  // namespace Kapua
