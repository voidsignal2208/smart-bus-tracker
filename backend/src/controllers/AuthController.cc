#include "AuthController.h"
#include "../services/AuthService.h"

using namespace drogon;

void AuthController::login(const HttpRequestPtr& req,
                           std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["email"].isString() || !(*json)["password"].isString()) {
        Json::Value ret;
        ret["error"] = "Invalid JSON credentials";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    AuthService::login((*json)["email"].asString(), (*json)["password"].asString(), callback);
}

void AuthController::registerUser(const HttpRequestPtr& req,
                                   std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json ||
        !(*json)["name"].isString() ||
        !(*json)["email"].isString() ||
        !(*json)["password"].isString() ||
        !(*json)["role"].isString()) {
        Json::Value ret;
        ret["error"] = "name, email, password and role are required";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    std::string phone = (*json)["phone"].isString() ? (*json)["phone"].asString() : "";

    AuthService::registerUser((*json)["name"].asString(),
                               (*json)["email"].asString(),
                               (*json)["password"].asString(),
                               (*json)["role"].asString(),
                               phone,
                               callback);
}
