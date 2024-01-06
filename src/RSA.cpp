//
// Kapua RSA class
//
// Author: Tom Cully <mail@tomcully.com>
// Copyright (c) Tom Cully 2023
//
#include "RSA.hpp"

#include <openssl/core_names.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>

namespace Kapua {

RSA::RSA(Logger* logger, Config* config) {
  _logger = new ScopedLogger("RSA", logger);
  _config = config;
}

RSA::~RSA() { delete _logger; }

bool RSA::generate_rsa_key_pair(const std::string& publicKeyFile, const std::string& privateKeyFile, int keyBits) {
  OpenSSL_add_all_algorithms();
  OPENSSL_init_crypto(0, nullptr);

  EVP_PKEY* rsaKey = EVP_PKEY_new();
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);

  if (EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keyBits) <= 0 || EVP_PKEY_keygen(ctx, &rsaKey) <= 0) {
    _logger->error("Error generating RSA key pair.");
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(rsaKey);
    return false;
  }

  // Save the public key to a PEM file
  FILE* pubKeyFile = fopen(publicKeyFile.c_str(), "wb");
  if (pubKeyFile) {
    PEM_write_PUBKEY(pubKeyFile, rsaKey);
    fclose(pubKeyFile);
  } else {
    _logger->error("Error saving public key to PEM file.");
    EVP_PKEY_free(rsaKey);
    EVP_PKEY_CTX_free(ctx);
    return false;
  }

  // Save the private key to a PEM file
  FILE* privKeyFile = fopen(privateKeyFile.c_str(), "wb");
  if (privKeyFile) {
    PEM_write_PrivateKey(privKeyFile, rsaKey, nullptr, nullptr, 0, nullptr, nullptr);
    fclose(privKeyFile);
  } else {
    _logger->error("Error saving private key to PEM file.");
    EVP_PKEY_free(rsaKey);
    EVP_PKEY_CTX_free(ctx);
    return false;
  }

  // Clean up
  EVP_PKEY_free(rsaKey);
  EVP_PKEY_CTX_free(ctx);

  return true;
}

bool RSA::load_rsa_key_pair(const std::string& publicKeyFile, const std::string& privateKeyFile, KeyPair& keyPair) {
  bool success = true;

  keyPair.publicKey = nullptr;
  keyPair.privateKey = nullptr;

  // Load the public key from PEM file
  FILE* pubKeyFile = fopen(publicKeyFile.c_str(), "rb");
  if (!pubKeyFile) {
    _logger->error("Error opening public key PEM file.");
    success = false;
  } else {
    keyPair.publicKey = PEM_read_PUBKEY(pubKeyFile, nullptr, nullptr, nullptr);
    fclose(pubKeyFile);
    if (!keyPair.publicKey) {
      _logger->error("Error loading public key from PEM file.");
    }
  }

  // Load the private key from PEM file
  FILE* privKeyFile = fopen(privateKeyFile.c_str(), "rb");
  if (!privKeyFile) {
    _logger->error("Error opening private key PEM file.");
    success = false;
  } else {
    keyPair.privateKey = PEM_read_PrivateKey(privKeyFile, nullptr, nullptr, nullptr);
    fclose(privKeyFile);
    if (!keyPair.privateKey) {
      _logger->error("Error loading private key from PEM file.");
      return false;
    }
  }

  if (!success) {
    if (keyPair.publicKey) EVP_PKEY_free(keyPair.publicKey);
    if (keyPair.privateKey) EVP_PKEY_free(keyPair.privateKey);
    keyPair.publicKey = nullptr;
    keyPair.privateKey = nullptr;
  }

  return success;
}

bool RSA::encrypt_aes_context(const AESContext* context, EVP_PKEY* publicKey, uint8_t* out_buffer, size_t in_size, size_t* out_size) {
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(publicKey, nullptr);

  if (!ctx || EVP_PKEY_encrypt_init(ctx) <= 0) {
    _logger->error("Error initializing encryption context.");
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return false;
  }

  // First call to determine the buffer size required for the encrypted data
  if (EVP_PKEY_encrypt(ctx, nullptr, out_size, reinterpret_cast<const uint8_t*>(context), sizeof(AESContext)) <= 0) {
    _logger->error("Error determining encrypted size.");
    EVP_PKEY_CTX_free(ctx);
    return false;
  }

  // Check if the provided buffer is large enough
  if (*out_size > in_size) {
    _logger->error("Provided buffer is too small for encrypted data.");
    EVP_PKEY_CTX_free(ctx);
    return false;
  }

  // Actual encryption
  if (EVP_PKEY_encrypt(ctx, out_buffer, out_size, reinterpret_cast<const uint8_t*>(context), sizeof(AESContext)) <= 0) {
    _logger->error("Error encrypting context.");
    EVP_PKEY_CTX_free(ctx);
    return false;
  }

  EVP_PKEY_CTX_free(ctx);
  return true;
}

bool RSA::decrypt_aes_context(AESContext* context, EVP_PKEY* privateKey, const uint8_t* in_buffer, size_t in_size, size_t* out_size) {
  EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privateKey, nullptr);
  if (!ctx || EVP_PKEY_decrypt_init(ctx) <= 0) {
    _logger->error("Error initializing decryption context.");
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return false;
  }

  if (EVP_PKEY_decrypt(ctx, reinterpret_cast<uint8_t*>(context), out_size, in_buffer, in_size) <= 0) {
    _logger->error("Error decrypting context.");
    EVP_PKEY_CTX_free(ctx);
    return false;
  }

  if (*out_size != sizeof(AESContext)) {
    _logger->error("Decrypted size mismatch with AESContext.");
    EVP_PKEY_CTX_free(ctx);
    return false;
  }

  EVP_PKEY_CTX_free(ctx);
  return true;
}

size_t RSA::get_pkey_size(EVP_PKEY* pKey) {
  if (pKey == nullptr) {
    // Handle the error or return 0, depending on how you want to handle null pointers
    return 0;
  }
  return EVP_PKEY_size(pKey);
}

}  // namespace Kapua
