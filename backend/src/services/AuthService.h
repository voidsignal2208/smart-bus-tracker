#pragma once

#include <drogon/HttpRequest.h>
#include <drogon/HttpResponse.h>
#include <functional>



class AuthService
{
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    
    static void registerUser(const std::string& name,
                              const std::string& email,
                              const std::string& password,
                              int roleId,
                              ResponseCallback callback);

    
    static void login(const std::string& email,
                       const std::string& password,
                       ResponseCallback callback);
};
