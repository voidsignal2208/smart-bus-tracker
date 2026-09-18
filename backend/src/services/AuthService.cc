#include "AuthService.h"

#include "../models/Users.h"
#include "../utils/JwtUtils.h"
#include "../utils/PasswordUtils.h"
#include "../utils/Roles.h"
#include "../utils/ValidationUtils.h"

#include <drogon/drogon.h>
#include <algorithm>
#include <cctype>

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
}  // namespace

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

void AuthService::registerUser(const std::string& name,
                                const std::string& email,
                                const std::string& password,
                                const std::string& role,
                                const std::string& phone,
                                ResponseCallback callback)
{
    if (name.empty() || name.size() > 100)
    {
        callback(jsonError(k400BadRequest, "Name is required and must be under 100 characters"));
        return;
    }

    if (!ValidationUtils::isValidEmail(email))
    {
        callback(jsonError(k400BadRequest, "Invalid email address"));
        return;
    }

    if (password.size() < 8)
    {
        callback(jsonError(k400BadRequest, "Password must be at least 8 characters long"));
        return;
    }

    std::string roleUpper = role;
    std::transform(roleUpper.begin(), roleUpper.end(), roleUpper.begin(),
                    [](unsigned char c) { return std::toupper(c); });

    int roleId = 0;
    if (roleUpper == "DRIVER")
    {
        roleId = Roles::DRIVER;
    }
    else if (roleUpper == "ADMIN")
    {
        roleId = Roles::ADMIN;
    }
    else
    {
        callback(jsonError(k400BadRequest, "Role must be DRIVER or ADMIN"));
        return;
    }

    std::string passwordHash;
    try
    {
        passwordHash = PasswordUtils::hashPassword(password);
    }
    catch (const std::exception&)
    {
        callback(jsonError(k500InternalServerError, "Failed to process password"));
        return;
    }

    auto dbClient = app().getDbClient();
    orm::Mapper<Users> mapper(dbClient);

    mapper.findBy(orm::Criteria(Users::Cols::_email, orm::CompareOperator::EQ, email),
        [callback, dbClient, name, email, passwordHash, phone, roleId](const std::vector<Users>& users) {
            if (!users.empty())
            {
                callback(jsonError(k409Conflict, "An account with this email already exists"));
                return;
            }

            Users newUser;
            newUser.setName(name);
            newUser.setEmail(email);
            newUser.setPasswordHash(passwordHash);
            newUser.setRoleId(roleId);
            if (!phone.empty())
            {
                newUser.setPhone(phone);
            }

            orm::Mapper<Users> insertMapper(dbClient);
            insertMapper.insert(newUser,
                [callback](Users inserted) {
                    std::string token = JwtUtils::generateToken(inserted.getValueOfId(), inserted.getValueOfRoleId());

                    Json::Value ret;
                    ret["message"] = "Account created successfully";
                    ret["token"] = token;
                    ret["role_id"] = inserted.getValueOfRoleId();
                    auto resp = HttpResponse::newHttpJsonResponse(ret);
                    resp->setStatusCode(k201Created);
                    callback(resp);
                },
                [callback](const orm::DrogonDbException& e) {
                    std::string what = e.base().what();
                    if (what.find("duplicate") != std::string::npos ||
                        what.find("unique") != std::string::npos)
                    {
                        callback(jsonError(k409Conflict, "An account with this email already exists"));
                    }
                    else
                    {
                        callback(jsonError(k500InternalServerError, "Database error during registration"));
                    }
                });
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k500InternalServerError, "Database error during registration"));
        });
}
