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
}  // namespace

// ----------------------------------------------------------------------
// API: POST /api/v1/routes
// Body: { "name": "Airport Express", "origin": "Downtown", "destination": "Terminal 1" }
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------
// API: GET /api/v1/routes
// ----------------------------------------------------------------------
void RouteController::getAllRoutes(const HttpRequestPtr& req,
                                   std::function<void(const HttpResponsePtr&)>&& callback) {
    RouteService::getAllRoutes(callback);
}

// ----------------------------------------------------------------------
// API: POST /api/v1/routes/{routeId}/stops
// Body: { "name": "Central Station", "latitude": 40.7128, "longitude": -74.0060, "sequence_order": 1 }
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------
// API: GET /api/v1/routes/{routeId}/stops
// ----------------------------------------------------------------------
void RouteController::getRouteStops(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback,
                                    std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    RouteService::getRouteStops(routeId, callback);
}
