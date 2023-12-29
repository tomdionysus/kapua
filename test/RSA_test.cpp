#include "RSA.hpp"

#include <gtest/gtest.h>
#include <fstream>
#include <memory>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <iostream>

#include "Logger.hpp"
#include "Config.hpp"

#include "MockLogger.hpp" // Correct path to MockLogger if necessary

using namespace Kapua;

class RSATest : public ::testing::Test {
 protected:
  void SetUp() override {
    mockLogger = std::make_unique<MockLogger>();
    rsa = std::make_unique<Kapua::RSA>(mockLogger.get(), nullptr);
  }

  bool CheckFileExists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
  }

  AESContext generateRandomAESContext() {
    AESContext context;
    RAND_bytes(context.iv, sizeof(context.iv));
    RAND_bytes(context.session_key, sizeof(context.session_key));
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
  ASSERT_TRUE(CheckFileExists("fixtures/public.pem"));
  ASSERT_TRUE(CheckFileExists("fixtures/private.pem"));

  KeyPair keyPair;
  keyPair.publicKey = nullptr;
  keyPair.privateKey = nullptr;
  bool result = rsa->loadRSAKeyPair("fixtures/public.pem", "fixtures/private.pem", keyPair);
  EXPECT_TRUE(result);

  if (keyPair.publicKey) EVP_PKEY_free(keyPair.publicKey);
  if (keyPair.privateKey) EVP_PKEY_free(keyPair.privateKey);
}

TEST_F(RSATest, LoadKeyPairWithNonExistentPublicKey) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening public key PEM file."));
  KeyPair keyPair;
  keyPair.publicKey = nullptr;
  keyPair.privateKey = nullptr;
  bool result = rsa->loadRSAKeyPair("fixtures/non_existent_public.pem", "fixtures/private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, LoadKeyPairWithNonExistentPrivateKey) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening private key PEM file."));
  KeyPair keyPair;
  bool result = rsa->loadRSAKeyPair("../test/fixtures/public.pem", "../test/fixtures/non_existent_private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, LoadKeyPairWithInvalidPublicKeyFormat) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening public key PEM file."));
  KeyPair keyPair;
  bool result = rsa->loadRSAKeyPair("../test/fixtures/invalid_format_public.pem", "../test/fixtures/private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, LoadKeyPairWithInvalidPrivateKeyFormat) {
  EXPECT_CALL(*mockLogger, error("(RSA) Error opening private key PEM file."));
  KeyPair keyPair;
  bool result = rsa->loadRSAKeyPair("../test/fixtures/public.pem", "../test/fixtures/invalid_format_private.pem", keyPair);
  EXPECT_FALSE(result);
}

TEST_F(RSATest, EncryptAndDecryptAESContext) {
  // Load RSA key pair
  KeyPair keyPair;
  ASSERT_TRUE(rsa->loadRSAKeyPair("fixtures/public.pem", "fixtures/private.pem", keyPair));

  // Generate a random AESContext
  AESContext originalContext = generateRandomAESContext();

  // Allocate buffer for encryption (2048 bits / 8 bits per byte)
  const size_t bufferSize = 2048 / 8;
  uint8_t buffer[bufferSize];
  size_t encryptedSize;

  // Encrypt the AESContext with the public key
  ASSERT_TRUE(rsa->encryptContext(&originalContext, keyPair.publicKey, buffer, bufferSize, &encryptedSize));

  // Check the encrypted is different
  ASSERT_NE(encryptedSize, 0);
  ASSERT_NE(memcmp(&originalContext, buffer, sizeof(AESContext)), 0);

  // Decrypt the encrypted AESContext
  AESContext decryptedContext;

  size_t decryptedSize;
  ASSERT_TRUE(rsa->decryptContext(&decryptedContext, keyPair.privateKey, buffer, encryptedSize, &decryptedSize));

  // Compare the decrypted context with the original
  ASSERT_EQ(memcmp(&originalContext, &decryptedContext, sizeof(AESContext)), 0);

  // Clean up
  EVP_PKEY_free(keyPair.publicKey);
  EVP_PKEY_free(keyPair.privateKey);
}




