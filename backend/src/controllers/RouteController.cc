#include "RouteController.h"
#include "../services/RouteService.h"
#include "../utils/ValidationUtils.h"

using namespace drogon;

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





void RouteController::createRoute(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["name"].isString() || !(*json)["origin"].isString() || !(*json)["destination"].isString()) {
        callback(jsonError(k400BadRequest, "Missing route data (name, origin, destination required)"));
        return;
    }

    RouteService::createRoute((*json)["name"].asString(),
                               (*json)["origin"].asString(),
                               (*json)["destination"].asString(),
                               callback);
}




void RouteController::getAllRoutes(const HttpRequestPtr& req,
                                   std::function<void(const HttpResponsePtr&)>&& callback) {
    RouteService::getAllRoutes(callback);
}





void RouteController::addStop(const HttpRequestPtr& req,
                              std::function<void(const HttpResponsePtr&)>&& callback,
                              std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !(*json)["name"].isString() || !(*json)["latitude"].isNumeric() ||
        !(*json)["longitude"].isNumeric() || !(*json)["sequence_order"].isInt()) {
        callback(jsonError(k400BadRequest, "Invalid stop data"));
        return;
    }

    RouteService::addStop(routeId,
                           (*json)["name"].asString(),
                           (*json)["latitude"].asDouble(),
                           (*json)["longitude"].asDouble(),
                           (*json)["sequence_order"].asInt(),
                           callback);
}




void RouteController::getRouteStops(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback,
                                    std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    RouteService::getRouteStops(routeId, callback);
}





void RouteController::getRouteBuses(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback,
                                    std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    RouteService::getRouteBuses(routeId, callback);
}
