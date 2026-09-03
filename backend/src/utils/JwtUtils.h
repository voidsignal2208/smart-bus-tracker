#pragma once










#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <string>
#include <chrono>
#include <optional>

#include "EnvConfig.h"


struct AuthClaims
{
    std::string userId;
    int roleId;
};

class JwtUtils
{
private:
    static constexpr int EXPIRATION_HOURS = 24;

    
    
    
    static const std::string& secretKey()
    {
        static const std::string secret = EnvConfig::require("JWT_SECRET");
        return secret;
    }

public:
    
    static std::string generateToken(const std::string& userId, int roleId)
    {
        auto now = std::chrono::system_clock::now();
        auto expiresAt = now + std::chrono::hours(EXPIRATION_HOURS);

        return jwt::create()
            .set_issuer("SmartBusTracker")
            .set_type("JWS")
            .set_payload_claim("user_id", jwt::claim(userId))
            .set_payload_claim("role_id", jwt::claim(std::to_string(roleId)))
            .set_issued_at(now)
            .set_expires_at(expiresAt)
            .sign(jwt::algorithm::hs256{secretKey()});
    }

    
    
    static std::optional<AuthClaims> verifyToken(const std::string& token)
    {
        try
        {
            auto decoded = jwt::decode(token);
            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{secretKey()})
                .with_issuer("SmartBusTracker");

            verifier.verify(decoded);

            AuthClaims claims;
            claims.userId = decoded.get_payload_claim("user_id").as_string();
            claims.roleId = std::stoi(decoded.get_payload_claim("role_id").as_string());
            return claims;
        }
        catch (const std::exception&)
        {
            return std::nullopt;
        }
    }

    
    static std::string verifyTokenAndGetUserId(const std::string& token)
    {
        auto claims = verifyToken(token);
        return claims.has_value() ? claims->userId : "";
    }
};
