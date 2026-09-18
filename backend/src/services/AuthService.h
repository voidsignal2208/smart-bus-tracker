#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <functional>



class AuthService
{
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    // Verifies credentials and, on success, returns a signed JWT.
    static void login(const std::string& email,
                       const std::string& password,
                       ResponseCallback callback);

    static void registerUser(const std::string& name,
                              const std::string& email,
                              const std::string& password,
                              const std::string& role,
                              const std::string& phone,
                              ResponseCallback callback);
};
