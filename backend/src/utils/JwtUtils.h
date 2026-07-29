#pragma once

// vcpkg's jwt-cpp port ships without the bundled picojson headers (it
// deletes include/picojson before installing), so plain <jwt-cpp/jwt.h>
// won't compile against it — jwt.h unconditionally tries to
// #include "picojson/picojson.h" unless JWT_DISABLE_PICOJSON is already
// defined. Including the nlohmann-json traits defaults header instead
// defines JWT_DISABLE_PICOJSON before pulling in jwt.h, and provides the
// same bare jwt::create()/decode()/verify()/claim API we already use,
// backed by nlohmann::json (which we depend on anyway). No call sites
// below need to change.
#include <jwt-cpp/traits/nlohmann-json/defaults.h>
#include <string>
#include <chrono>
#include <optional>

#include "EnvConfig.h"

// Result of successfully verifying a token.
struct AuthClaims
{
    std::string userId;
    int roleId;
};

class JwtUtils
{
private:
    static constexpr int EXPIRATION_HOURS = 24;

    // Secret is read from the JWT_SECRET environment variable. Cached after
    // first use. Loaded lazily (rather than as a static initializer) so
    // that main() has a chance to call EnvConfig::loadDotEnv() first.
    static const std::string& secretKey()
    {
        static const std::string secret = EnvConfig::require("JWT_SECRET");
        return secret;
    }

public:
    // Generate a JWT for a specific user ID and Role
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

    // Verifies a token and returns its claims, or std::nullopt if the
    // token is invalid, expired, or malformed.
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

    // Convenience wrapper kept for callers that only need the user id.
    static std::string verifyTokenAndGetUserId(const std::string& token)
    {
        auto claims = verifyToken(token);
        return claims.has_value() ? claims->userId : "";
    }
};
