#pragma once

#include <string>

// Secure password hashing using PBKDF2-HMAC-SHA256 with a random salt
// per password, implemented via OpenSSL's EVP API (no extra dependency
// beyond the OpenSSL that's already linked for JWT support).
//
// Stored format: pbkdf2-sha256$<iterations>$<salt-hex>$<hash-hex>
// Storing the algorithm/iteration count/salt alongside the hash lets us
// verify old hashes even if we bump the iteration count later.
class PasswordUtils
{
public:
    // Hashes a plaintext password with a freshly generated random salt.
    static std::string hashPassword(const std::string& password);

    // Verifies a plaintext password against a stored hash produced by
    // hashPassword(). Returns false (never throws) on any malformed input.
    static bool verifyPassword(const std::string& password, const std::string& storedHash);

private:
    static constexpr int kIterations = 210000; // OWASP-recommended minimum (2023) for PBKDF2-HMAC-SHA256
    static constexpr int kSaltBytes = 16;
    static constexpr int kKeyBytes = 32;

    static std::string toHex(const unsigned char* data, size_t len);
    static std::string fromHexToBytes(const std::string& hex);
};
