#include "AuthController.h"
#include "../services/AuthService.h"

using namespace drogon;

void AuthController::registerUser(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["name"].isString() || !(*json)["email"].isString() || !(*json)["password"].isString()) {
        Json::Value ret;
        ret["error"] = "Invalid JSON or missing fields (name, email, password required)";
        auto resp = HttpResponse::newHttpJsonResponse(ret);
        resp->setStatusCode(k400BadRequest);
        callback(resp);
        return;
    }

    int roleId = (*json)["role_id"].isInt() ? (*json)["role_id"].asInt() : 0;

    AuthService::registerUser((*json)["name"].asString(),
                               (*json)["email"].asString(),
                               (*json)["password"].asString(),
                               roleId,
                               callback);
}

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
