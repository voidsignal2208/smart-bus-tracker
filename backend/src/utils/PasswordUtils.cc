#include "PasswordUtils.h"

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <iomanip>
#include <sstream>
#include <vector>
#include <sstream>
#include <cstring>

std::string PasswordUtils::toHex(const unsigned char* data, size_t len)
{
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
    {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return oss.str();
}

std::string PasswordUtils::fromHexToBytes(const std::string& hex)
{
    std::string bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i + 1 < hex.size(); i += 2)
    {
        unsigned int byte = 0;
        std::istringstream(hex.substr(i, 2)) >> std::hex >> byte;
        bytes.push_back(static_cast<char>(byte));
    }
    return bytes;
}

std::string PasswordUtils::hashPassword(const std::string& password)
{
    unsigned char salt[kSaltBytes];
    if (RAND_bytes(salt, kSaltBytes) != 1)
    {
        throw std::runtime_error("Failed to generate random salt for password hashing");
    }

    unsigned char derivedKey[kKeyBytes];
    int ok = PKCS5_PBKDF2_HMAC(
        password.c_str(), static_cast<int>(password.size()),
        salt, kSaltBytes,
        kIterations,
        EVP_sha256(),
        kKeyBytes, derivedKey);

    if (ok != 1)
    {
        throw std::runtime_error("PBKDF2 hashing failed");
    }

    std::ostringstream out;
    out << "pbkdf2-sha256$" << kIterations << "$"
        << toHex(salt, kSaltBytes) << "$"
        << toHex(derivedKey, kKeyBytes);
    return out.str();
}

bool PasswordUtils::verifyPassword(const std::string& password, const std::string& storedHash)
{
    
    size_t firstDollar = storedHash.find('$');
    if (firstDollar == std::string::npos) return false;

    size_t secondDollar = storedHash.find('$', firstDollar + 1);
    if (secondDollar == std::string::npos) return false;

    size_t thirdDollar = storedHash.find('$', secondDollar + 1);
    if (thirdDollar == std::string::npos) return false;

    std::string algo = storedHash.substr(0, firstDollar);
    if (algo != "pbkdf2-sha256") return false;

    int iterations = 0;
    try
    {
        iterations = std::stoi(storedHash.substr(firstDollar + 1, secondDollar - firstDollar - 1));
    }
    catch (...)
    {
        return false;
    }

    std::string saltHex = storedHash.substr(secondDollar + 1, thirdDollar - secondDollar - 1);
    std::string expectedHashHex = storedHash.substr(thirdDollar + 1);

    std::string salt = fromHexToBytes(saltHex);
    if (salt.empty() && !saltHex.empty()) return false;

    std::vector<unsigned char> derivedKey(kKeyBytes);
    int ok = PKCS5_PBKDF2_HMAC(
        password.c_str(), static_cast<int>(password.size()),
        reinterpret_cast<const unsigned char*>(salt.data()), static_cast<int>(salt.size()),
        iterations,
        EVP_sha256(),
        kKeyBytes, derivedKey.data());

    if (ok != 1) return false;

    std::string actualHashHex = toHex(derivedKey.data(), kKeyBytes);

    
    if (actualHashHex.size() != expectedHashHex.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < actualHashHex.size(); ++i)
    {
        diff |= static_cast<unsigned char>(actualHashHex[i] ^ expectedHashHex[i]);
    }
    return diff == 0;
}
