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

// ----------------------------------------------------------------------
// API: PUT /api/v1/routes/{routeId}
// Body: { "name": "Airport Express", "origin": "Downtown", "destination": "Terminal 1" }
// ----------------------------------------------------------------------
void RouteController::updateRoute(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback,
                                  std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }
    auto json = req->getJsonObject();
    if (!json || !(*json)["name"].isString() || !(*json)["origin"].isString() || !(*json)["destination"].isString()) {
        callback(jsonError(k400BadRequest, "name, origin, and destination are required"));
        return;
    }
    const std::string name = (*json)["name"].asString();
    const std::string origin = (*json)["origin"].asString();
    const std::string destination = (*json)["destination"].asString();
    if (name.empty() || name.size() > 100 || origin.empty() || origin.size() > 100 || destination.empty() || destination.size() > 100) {
        callback(jsonError(k400BadRequest, "Route fields must be between 1 and 100 characters"));
        return;
    }
    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "UPDATE routes SET name = $1, origin = $2, destination = $3 WHERE id = $4::uuid RETURNING id, name, origin, destination",
        [callback](const orm::Result& r) {
            if (r.empty()) {
                callback(jsonError(k404NotFound, "Route not found"));
                return;
            }
            Json::Value ret;
            ret["message"] = "Route updated successfully";
            ret["id"] = r[0]["id"].as<std::string>();
            ret["name"] = r[0]["name"].as<std::string>();
            ret["origin"] = r[0]["origin"].as<std::string>();
            ret["destination"] = r[0]["destination"].as<std::string>();
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to update route: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to update route"));
        },
        name, origin, destination, routeId);
}

// ----------------------------------------------------------------------
// API: DELETE /api/v1/routes/{routeId}
// Route stops cascade at the database level; assignments are removed first.
// ----------------------------------------------------------------------
void RouteController::deleteRoute(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback,
                                  std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }
    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "WITH removed_assignments AS (DELETE FROM bus_assignments WHERE route_id = $1::uuid) "
        "DELETE FROM routes WHERE id = $1::uuid RETURNING id",
        [callback](const orm::Result& r) {
            if (r.empty()) {
                callback(jsonError(k404NotFound, "Route not found"));
                return;
            }
            Json::Value ret;
            ret["message"] = "Route deleted successfully";
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to delete route: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to delete route"));
        },
        routeId);
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




void RouteController::getRouteStops(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback,
                                    std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    RouteService::getRouteStops(routeId, callback);
}

// API: PUT /api/v1/routes/{routeId}/stops/{stopId}
void RouteController::updateStop(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback,
                                 std::string routeId,
                                 std::string stopId) {
    if (!ValidationUtils::isValidUuid(routeId) || !ValidationUtils::isValidUuid(stopId)) {
        callback(jsonError(k400BadRequest, "routeId and stopId must be valid UUIDs"));
        return;
    }
    auto json = req->getJsonObject();
    if (!json || !(*json)["name"].isString() || !(*json)["latitude"].isNumeric() || !(*json)["longitude"].isNumeric()) {
        callback(jsonError(k400BadRequest, "name, latitude, and longitude are required"));
        return;
    }
    const std::string name = (*json)["name"].asString();
    const double latitude = (*json)["latitude"].asDouble();
    const double longitude = (*json)["longitude"].asDouble();
    if (name.empty() || name.size() > 100 || latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
        callback(jsonError(k400BadRequest, "Invalid stop data"));
        return;
    }
    app().getDbClient()->execSqlAsync(
        "UPDATE stops SET name = $1, latitude = $2, longitude = $3 WHERE id = $4::uuid AND route_id = $5::uuid RETURNING id",
        [callback](const orm::Result& r) {
            if (r.empty()) { callback(jsonError(k404NotFound, "Stop not found")); return; }
            Json::Value ret; ret["message"] = "Stop updated successfully";
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to update stop: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to update stop"));
        }, name, latitude, longitude, stopId, routeId);
}

// API: DELETE /api/v1/routes/{routeId}/stops/{stopId}
void RouteController::deleteStop(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback,
                                 std::string routeId,
                                 std::string stopId) {
    if (!ValidationUtils::isValidUuid(routeId) || !ValidationUtils::isValidUuid(stopId)) {
        callback(jsonError(k400BadRequest, "routeId and stopId must be valid UUIDs"));
        return;
    }
    app().getDbClient()->execSqlAsync(
        "WITH removed AS (DELETE FROM stops WHERE id = $1::uuid AND route_id = $2::uuid RETURNING sequence_order) "
        "UPDATE stops SET sequence_order = sequence_order - 1 FROM removed "
        "WHERE stops.route_id = $2::uuid AND stops.sequence_order > removed.sequence_order RETURNING stops.id",
        [callback](const orm::Result&) {
            Json::Value ret; ret["message"] = "Stop deleted successfully";
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to delete stop: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to delete stop"));
        }, stopId, routeId);
}

// API: PUT /api/v1/routes/{routeId}/stops/reorder
// Body: { "stop_ids": ["uuid", "uuid"] }
void RouteController::reorderStops(const HttpRequestPtr& req,
                                   std::function<void(const HttpResponsePtr&)>&& callback,
                                   std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }
    auto json = req->getJsonObject();
    if (!json || !(*json)["stop_ids"].isArray() || (*json)["stop_ids"].empty()) {
        callback(jsonError(k400BadRequest, "stop_ids must be a non-empty array"));
        return;
    }
    for (const auto& id : (*json)["stop_ids"]) {
        if (!id.isString() || !ValidationUtils::isValidUuid(id.asString())) {
            callback(jsonError(k400BadRequest, "Every stop_id must be a valid UUID"));
            return;
        }
    }
    Json::StreamWriterBuilder writer;
    const std::string idsJson = Json::writeString(writer, (*json)["stop_ids"]);
    app().getDbClient()->execSqlAsync(
        "WITH requested AS (SELECT value::uuid AS id, ordinality::int AS position "
        "FROM jsonb_array_elements_text($2::jsonb) WITH ORDINALITY), "
        "moved AS (UPDATE stops SET sequence_order = sequence_order + 1000000 WHERE route_id = $1::uuid) "
        "UPDATE stops s SET sequence_order = requested.position FROM requested "
        "WHERE s.route_id = $1::uuid AND s.id = requested.id RETURNING s.id",
        [callback](const orm::Result& r) {
            Json::Value ret; ret["message"] = "Stops reordered successfully"; ret["updated"] = static_cast<int>(r.size());
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to reorder stops: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to reorder stops"));
        }, routeId, idsJson);
}

// ----------------------------------------------------------------------
// API: POST /api/v1/routes/{routeId}/buses
// Body: { "bus_id": "uuid", "status": "SCHEDULED" }
// ----------------------------------------------------------------------
void RouteController::assignBus(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback,
                                std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !(*json)["bus_id"].isString()) {
        callback(jsonError(k400BadRequest, "bus_id is required"));
        return;
    }

    std::string status = (*json)["status"].isString() ? (*json)["status"].asString() : "SCHEDULED";
    RouteService::assignBus(routeId, (*json)["bus_id"].asString(), status, callback);
}

// ----------------------------------------------------------------------
// API: GET /api/v1/routes/{routeId}/buses?date=YYYY-MM-DD
// ----------------------------------------------------------------------
void RouteController::getRouteBuses(const HttpRequestPtr& req,
                                    std::function<void(const HttpResponsePtr&)>&& callback,
                                    std::string routeId) {
    if (!ValidationUtils::isValidUuid(routeId)) {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    // Read optional ?date= query param (format: YYYY-MM-DD)
    std::string date = req->getParameter("date");

    RouteService::getRouteBuses(routeId, date, callback);
}
