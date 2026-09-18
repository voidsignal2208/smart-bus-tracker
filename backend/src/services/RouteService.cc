#include "RouteService.h"

#include "../models/Routes.h"
#include "../models/Stops.h"
#include "../utils/ValidationUtils.h"

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

void RouteService::createRoute(const std::string& name,
                                const std::string& origin,
                                const std::string& destination,
                                ResponseCallback callback)
{
    if (name.empty() || name.size() > 100 || origin.empty() || destination.empty())
    {
        callback(jsonError(k400BadRequest, "name, origin, and destination are required"));
        return;
    }

    Routes newRoute;
    newRoute.setName(name);
    newRoute.setOrigin(origin);
    newRoute.setDestination(destination);

    auto dbClient = app().getDbClient();
    orm::Mapper<Routes> mapper(dbClient);

    mapper.insert(newRoute,
        [callback](const Routes& r) {
            Json::Value ret;
            ret["message"] = "Route created successfully";
            ret["route_id"] = r.getValueOfId();
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k201Created);
            callback(resp);
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k500InternalServerError, "Database error creating route"));
        });
}

void RouteService::getAllRoutes(ResponseCallback callback)
{
    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT id, name, origin, destination FROM routes ORDER BY name ASC",
        [callback](const drogon::orm::Result& r) {
            Json::Value ret(Json::arrayValue);
            for (const auto& row : r)
            {
                Json::Value rJson;
                rJson["id"] = row["id"].as<std::string>();
                rJson["name"] = row["name"].as<std::string>();
                rJson["origin"] = row["origin"].as<std::string>();
                rJson["destination"] = row["destination"].as<std::string>();
                ret.append(rJson);
            }
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to fetch routes: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to fetch routes"));
        });
}

void RouteService::addStop(const std::string& routeId,
                            const std::string& name,
                            double latitude,
                            double longitude,
                            int sequenceOrder,
                            ResponseCallback callback)
{
    if (name.empty() || name.size() > 100)
    {
        callback(jsonError(k400BadRequest, "Stop name must be between 1 and 100 characters"));
        return;
    }

    if (!ValidationUtils::isValidLatitude(latitude) || !ValidationUtils::isValidLongitude(longitude))
    {
        callback(jsonError(k400BadRequest, "latitude must be in [-90, 90] and longitude in [-180, 180]"));
        return;
    }

    if (sequenceOrder < 0)
    {
        callback(jsonError(k400BadRequest, "sequence_order must be a non-negative integer"));
        return;
    }

    Stops newStop;
    newStop.setRouteId(routeId);
    newStop.setName(name);
    newStop.setLatitude(std::to_string(latitude));
    newStop.setLongitude(std::to_string(longitude));
    newStop.setSequenceOrder(sequenceOrder);

    auto dbClient = app().getDbClient();
    orm::Mapper<Stops> mapper(dbClient);

    mapper.insert(newStop,
        [callback](const Stops& s) {
            Json::Value ret;
            ret["message"] = "Stop added successfully";
            ret["stop_id"] = s.getValueOfId();
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k201Created);
            callback(resp);
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k400BadRequest,
                                "Failed to add stop (route may not exist, or sequence_order is already used on this route)"));
        });
}

void RouteService::getRouteStops(const std::string& routeId, ResponseCallback callback)
{
    if (!ValidationUtils::isValidUuid(routeId))
    {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT id, name, latitude, longitude, sequence_order FROM stops WHERE route_id = $1::uuid ORDER BY sequence_order ASC",
        [callback](const drogon::orm::Result& r) {
            Json::Value ret(Json::arrayValue);
            for (const auto& row : r)
            {
                Json::Value sJson;
                sJson["id"] = row["id"].as<std::string>();
                sJson["name"] = row["name"].as<std::string>();
                sJson["latitude"] = std::stod(row["latitude"].as<std::string>());
                sJson["longitude"] = std::stod(row["longitude"].as<std::string>());
                sJson["sequence_order"] = row["sequence_order"].as<int>();
                ret.append(sJson);
            }
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to fetch stops: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to fetch stops"));
        },
        routeId);
}

void RouteService::getRouteBuses(const std::string& routeId, const std::string& date, ResponseCallback callback)
{
    if (!ValidationUtils::isValidUuid(routeId))
    {
        callback(jsonError(k400BadRequest, "routeId must be a valid UUID"));
        return;
    }

    auto dbClient = app().getDbClient();

    // Build query: filter by assignment_date if a date was provided
    if (!date.empty())
    {
        dbClient->execSqlAsync(
            "SELECT b.id, b.license_plate, b.capacity, b.status, "
            "COALESCE(b.rating, 0) AS rating, COALESCE(b.feedback_score, 0) AS feedback_score, "
            "ba.assignment_date::text AS assignment_date, ba.id AS assignment_id "
            "FROM buses b "
            "JOIN bus_assignments ba ON b.id = ba.bus_id "
            "WHERE ba.route_id = $1::uuid AND ba.assignment_date = $2::date",
            [callback](const drogon::orm::Result& r) {
                Json::Value ret(Json::arrayValue);
                for (const auto& row : r)
                {
                    Json::Value bJson;
                    bJson["id"] = row["id"].as<std::string>();
                    bJson["license_plate"] = row["license_plate"].as<std::string>();
                    bJson["capacity"] = row["capacity"].as<int>();
                    bJson["status"] = row["status"].as<std::string>();
                    bJson["rating"] = std::stod(row["rating"].as<std::string>());
                    bJson["feedback_score"] = std::stod(row["feedback_score"].as<std::string>());
                    if (!row["assignment_date"].isNull())
                        bJson["assignment_date"] = row["assignment_date"].as<std::string>();
                    if (!row["assignment_id"].isNull())
                        bJson["assignment_id"] = row["assignment_id"].as<std::string>();
                    ret.append(bJson);
                }
                callback(HttpResponse::newHttpJsonResponse(ret));
            },
            [callback](const drogon::orm::DrogonDbException& e) {
                LOG_ERROR << "getRouteBuses (date filter) error: " << e.base().what();
                callback(jsonError(k500InternalServerError, "Failed to fetch route buses"));
            },
            routeId, date);
    }
    else
    {
        dbClient->execSqlAsync(
            "SELECT b.id, b.license_plate, b.capacity, b.status, "
            "COALESCE(b.rating, 0) AS rating, COALESCE(b.feedback_score, 0) AS feedback_score, "
            "ba.assignment_date::text AS assignment_date, ba.id AS assignment_id "
            "FROM buses b "
            "JOIN bus_assignments ba ON b.id = ba.bus_id "
            "WHERE ba.route_id = $1::uuid",
            [callback](const drogon::orm::Result& r) {
                Json::Value ret(Json::arrayValue);
                for (const auto& row : r)
                {
                    Json::Value bJson;
                    bJson["id"] = row["id"].as<std::string>();
                    bJson["license_plate"] = row["license_plate"].as<std::string>();
                    bJson["capacity"] = row["capacity"].as<int>();
                    bJson["status"] = row["status"].as<std::string>();
                    bJson["rating"] = std::stod(row["rating"].as<std::string>());
                    bJson["feedback_score"] = std::stod(row["feedback_score"].as<std::string>());
                    if (!row["assignment_date"].isNull())
                        bJson["assignment_date"] = row["assignment_date"].as<std::string>();
                    if (!row["assignment_id"].isNull())
                        bJson["assignment_id"] = row["assignment_id"].as<std::string>();
                    ret.append(bJson);
                }
                callback(HttpResponse::newHttpJsonResponse(ret));
            },
            [callback](const drogon::orm::DrogonDbException& e) {
                LOG_ERROR << "getRouteBuses error: " << e.base().what();
                callback(jsonError(k500InternalServerError, "Failed to fetch route buses"));
            },
            routeId);
    }
}

void RouteService::assignBus(const std::string& routeId,
                             const std::string& busId,
                             const std::string& status,
                             ResponseCallback callback)
{
    if (!ValidationUtils::isValidUuid(routeId) || !ValidationUtils::isValidUuid(busId))
    {
        callback(jsonError(k400BadRequest, "routeId and busId must be valid UUIDs"));
        return;
    }

    auto dbClient = app().getDbClient();
    std::string effectiveStatus = status.empty() ? "SCHEDULED" : status;
    dbClient->execSqlAsync(
        "INSERT INTO bus_assignments (bus_id, route_id, status, assignment_date) "
        "VALUES ($1::uuid, $2::uuid, $3, CURRENT_DATE) RETURNING id",
        [callback](const drogon::orm::Result& r) {
            Json::Value ret;
            ret["message"] = "Bus assigned successfully";
            if (!r.empty())
            {
                ret["assignment_id"] = r[0]["id"].as<std::string>();
            }
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k201Created);
            callback(resp);
        },
        [callback](const drogon::orm::DrogonDbException& e) {
            LOG_ERROR << "assignBus error: " << e.base().what();
            callback(jsonError(k400BadRequest, "Failed to assign bus (bus or route may not exist)"));
        },
        busId, routeId, effectiveStatus);
}
