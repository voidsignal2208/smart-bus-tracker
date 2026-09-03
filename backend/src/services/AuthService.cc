#include "AuthService.h"

#include "../models/Users.h"
#include "../utils/JwtUtils.h"
#include "../utils/PasswordUtils.h"
#include "../utils/ValidationUtils.h"
#include "../utils/Roles.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon_model::postgres;

namespace
{
HttpResponsePtr jsonError(HttpStatusCode code, const std::string& message)
{
    Json::Value ret;
    ret["error"] = message;
    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(code);
    return resp;
}
}  

void AuthService::registerUser(const std::string& name,
                                const std::string& email,
                                const std::string& password,
                                int roleId,
                                ResponseCallback callback)
{
    if (name.empty() || name.size() > 100)
    {
        callback(jsonError(k400BadRequest, "Name must be between 1 and 100 characters"));
        return;
    }

    if (!ValidationUtils::isValidEmail(email))
    {
        callback(jsonError(k400BadRequest, "Invalid email address"));
        return;
    }

    if (password.size() < 8)
    {
        callback(jsonError(k400BadRequest, "Password must be at least 8 characters"));
        return;
    }

    
    
    
    
    (void)roleId;
    int effectiveRoleId = Roles::PASSENGER;

    Users newUser;
    newUser.setName(name);
    newUser.setEmail(email);
    newUser.setPasswordHash(PasswordUtils::hashPassword(password));
    newUser.setRoleId(effectiveRoleId);

    auto dbClient = app().getDbClient();
    orm::Mapper<Users> mapper(dbClient);

    mapper.insert(newUser,
        [callback](const Users& u) {
            Json::Value ret;
            ret["message"] = "User registered successfully";
            ret["user_id"] = u.getValueOfId();
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k201Created);
            callback(resp);
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k409Conflict, "A user with that email already exists"));
        });
}

void AuthService::login(const std::string& email,
                         const std::string& password,
                         ResponseCallback callback)
{
    if (!ValidationUtils::isValidEmail(email) || password.empty())
    {
        callback(jsonError(k401Unauthorized, "Invalid email or password"));
        return;
    }

    auto dbClient = app().getDbClient();
    orm::Mapper<Users> mapper(dbClient);

    mapper.findBy(orm::Criteria(Users::Cols::_email, orm::CompareOperator::EQ, email),
        [callback, password](const std::vector<Users>& users) {
            if (users.empty())
            {
                callback(jsonError(k401Unauthorized, "Invalid email or password"));
                return;
            }

            const auto& user = users.front();

            if (!PasswordUtils::verifyPassword(password, user.getValueOfPasswordHash()))
            {
                callback(jsonError(k401Unauthorized, "Invalid email or password"));
                return;
            }

            std::string token = JwtUtils::generateToken(user.getValueOfId(), user.getValueOfRoleId());

            Json::Value ret;
            ret["message"] = "Login successful";
            ret["token"] = token;
            ret["role_id"] = user.getValueOfRoleId();

            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k500InternalServerError, "Database error during login"));
        });
}
