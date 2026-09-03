#pragma once

#include <string>








class PasswordUtils
{
public:
    
    static std::string hashPassword(const std::string& password);

    
    
    static bool verifyPassword(const std::string& password, const std::string& storedHash);

private:
    static constexpr int kIterations = 210000; 
    static constexpr int kSaltBytes = 16;
    static constexpr int kKeyBytes = 32;

    static std::string toHex(const unsigned char* data, size_t len);
    static std::string fromHexToBytes(const std::string& hex);
};
