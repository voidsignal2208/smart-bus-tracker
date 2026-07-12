#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <functional>

// Encapsulates the registration/login business logic so AuthController
// stays a thin HTTP adapter.
class AuthService
{
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    // Validates input, hashes the password, and inserts a new user.
    static void registerUser(const std::string& name,
                              const std::string& email,
                              const std::string& password,
                              int roleId,
                              ResponseCallback callback);

    // Verifies credentials and, on success, returns a signed JWT.
    static void login(const std::string& email,
                       const std::string& password,
                       ResponseCallback callback);
};
