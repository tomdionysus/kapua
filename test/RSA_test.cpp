#include "RSA.hpp"

#include <gtest/gtest.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#include <fstream>
#include <iostream>
#include <memory>

#include "Config.hpp"
#include "Logger.hpp"
#include "MockLogger.hpp"  // Correct path to MockLogger if necessary

using namespace Kapua;

namespace KapuaTest {

class RSATest : public ::testing::Test {
 protected:
  void SetUp() override {
    mockLogger = std::make_unique<MockLogger>();
    rsa = std::make_unique<Kapua::RSA>(mockLogger.get(), nullptr);
  }

  bool check_file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
  }

  AESKey generate_random_aes_context() {
    AESKey context;
    RAND_bytes(context.key, sizeof(context.key));
    return context;
  }

  std::unique_ptr<MockLogger> mockLogger;
  std::unique_ptr<Kapua::RSA> rsa;
};

TEST_F(RSATest, CheckLoggerMock) {
  EXPECT_CALL(*mockLogger, raw("Testing Logger Mock"));
  mockLogger->raw("Testing Logger Mock");
}

TEST_F(RSATest, LoadValidKeyPair) {
  ASSERT_TRUE(check_file_exists("fixtures/public.pem"));
  ASSERT_TRUE(check_file_exists("fixtures/private.pem"));

  KeyPair keyPair;
  keyPair.publicKey = nullptr;
  keyPair.privateKey = nullptr;
  bool result = rsa->load_rsa_key_pair("fixtures/public.pem", "fixtures/private.pem", keyPair);
  EXPECT_TRUE(result);

  if (keyPair.publicKey) EVP_PKEY_free(keyPair.publicKey);
  if (keyPair.privateKey) EVP_PKEY_free(keyPair.privateKey);
}

TEST_F(RSATest, LoadKeyPairWithNonExistentPublicKey) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening public key PEM file."));
  KeyPair keyPair;
  keyPair.publicKey = nullptr;
  keyPair.privateKey = nullptr;
  bool result = rsa->load_rsa_key_pair("fixtures/non_existent_public.pem", "fixtures/private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, LoadKeyPairWithNonExistentPrivateKey) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening private key PEM file."));
  KeyPair keyPair;
  bool result = rsa->load_rsa_key_pair("../test/fixtures/public.pem", "../test/fixtures/non_existent_private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, LoadKeyPairWithInvalidPublicKeyFormat) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening public key PEM file."));
  KeyPair keyPair;
  bool result = rsa->load_rsa_key_pair("../test/fixtures/invalid_format_public.pem", "../test/fixtures/private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, LoadKeyPairWithInvalidPrivateKeyFormat) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening private key PEM file."));
  KeyPair keyPair;
  bool result = rsa->load_rsa_key_pair("../test/fixtures/public.pem", "../test/fixtures/invalid_format_private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, EncryptAndDecryptAESContext) {
  // Load RSA key pair
  KeyPair keyPair;
  ASSERT_TRUE(rsa->load_rsa_key_pair("fixtures/public.pem", "fixtures/private.pem", keyPair));

  // Generate a random AESKey
  AESKey originalContext = generate_random_aes_context();

  // Allocate buffer for encryption (2048 bits / 8 bits per byte)
  const size_t bufferSize = 2048 / 8;
  uint8_t buffer[bufferSize];
  size_t encryptedSize;

  // Encrypt the AESKey with the public key
  ASSERT_TRUE(rsa->encrypt_aes_context(&originalContext, keyPair.publicKey, buffer, bufferSize, &encryptedSize));

  // Check the encrypted is different
  ASSERT_NE(encryptedSize, 0);
  ASSERT_NE(memcmp(&originalContext, buffer, sizeof(AESKey)), 0);

  // Decrypt the encrypted AESKey
  AESKey decryptedContext;

  size_t decryptedSize;
  ASSERT_TRUE(rsa->decrypt_aes_context(&decryptedContext, keyPair.privateKey, buffer, encryptedSize, &decryptedSize));

  // Compare the decrypted context with the original
  ASSERT_EQ(decryptedSize, sizeof(AESKey));
  ASSERT_EQ(memcmp(&originalContext, &decryptedContext, sizeof(AESKey)), 0);

  // Clean up
  EVP_PKEY_free(keyPair.publicKey);
  EVP_PKEY_free(keyPair.privateKey);
}

}  // namespace KapuaTest
