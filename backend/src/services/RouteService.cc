#include "RouteService.h"

#include "../models/Routes.h"
#include "../models/Stops.h"
#include "../models/BusAssignments.h"
#include "../models/Buses.h"
#include "../utils/ValidationUtils.h"

#include <drogon/drogon.h>
#include <map>
#include <vector>

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
    orm::Mapper<Routes> mapper(dbClient);

    mapper.findAll(
        [callback](const std::vector<Routes>& routes) {
            Json::Value ret(Json::arrayValue);
            for (const auto& route : routes)
            {
                Json::Value rJson;
                rJson["id"] = route.getValueOfId();
                rJson["name"] = route.getValueOfName();
                rJson["origin"] = route.getValueOfOrigin();
                rJson["destination"] = route.getValueOfDestination();
                ret.append(rJson);
            }
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
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
    auto dbClient = app().getDbClient();
    orm::Mapper<Stops> mapper(dbClient);

    mapper.orderBy(Stops::Cols::_sequence_order, orm::SortOrder::ASC).findBy(
        orm::Criteria(Stops::Cols::_route_id, orm::CompareOperator::EQ, routeId),
        [callback](const std::vector<Stops>& stops) {
            Json::Value ret(Json::arrayValue);
            for (const auto& stop : stops)
            {
                Json::Value sJson;
                sJson["id"] = stop.getValueOfId();
                sJson["name"] = stop.getValueOfName();
                sJson["latitude"] = std::stod(stop.getValueOfLatitude());
                sJson["longitude"] = std::stod(stop.getValueOfLongitude());
                sJson["sequence_order"] = stop.getValueOfSequenceOrder();
                ret.append(sJson);
            }
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k500InternalServerError, "Failed to fetch stops"));
        });
}







void RouteService::getRouteBuses(const std::string& routeId, ResponseCallback callback)
{
    auto dbClient = app().getDbClient();
    orm::Mapper<BusAssignments> assignmentMapper(dbClient);

    
    
    assignmentMapper.orderBy(BusAssignments::Cols::_assignment_date, orm::SortOrder::DESC).findBy(
        orm::Criteria(BusAssignments::Cols::_route_id, orm::CompareOperator::EQ, routeId) &&
        orm::Criteria(BusAssignments::Cols::_status, orm::CompareOperator::NE, std::string("CANCELLED")),
        [callback, dbClient](const std::vector<BusAssignments>& assignments) {
            if (assignments.empty())
            {
                callback(HttpResponse::newHttpJsonResponse(Json::Value(Json::arrayValue)));
                return;
            }

            
            
            
            std::vector<std::string> busIds;
            std::map<std::string, BusAssignments> assignmentByBusId;
            for (const auto& a : assignments)
            {
                const std::string busId = a.getValueOfBusId();
                if (busId.empty() || assignmentByBusId.count(busId)) continue;
                busIds.push_back(busId);
                assignmentByBusId.emplace(busId, a);
            }

            orm::Mapper<Buses> busMapper(dbClient);
            busMapper.findBy(
                orm::Criteria(Buses::Cols::_id, orm::CompareOperator::In, busIds),
                [callback, assignmentByBusId](const std::vector<Buses>& buses) {
                    Json::Value ret(Json::arrayValue);
                    for (const auto& bus : buses)
                    {
                        Json::Value bJson;
                        bJson["id"] = bus.getValueOfId();
                        bJson["license_plate"] = bus.getValueOfLicensePlate();
                        bJson["capacity"] = bus.getValueOfCapacity();
                        bJson["status"] = bus.getValueOfStatus();

                        auto it = assignmentByBusId.find(bus.getValueOfId());
                        if (it != assignmentByBusId.end())
                        {
                            bJson["assignment_status"] = it->second.getValueOfStatus();
                            bJson["driver_id"] = it->second.getValueOfDriverId();
                            bJson["conductor_id"] = it->second.getValueOfConductorId();
                        }
                        ret.append(bJson);
                    }
                    callback(HttpResponse::newHttpJsonResponse(ret));
                },
                [callback](const orm::DrogonDbException& e) {
                    callback(jsonError(k500InternalServerError, "Failed to fetch buses for route"));
                });
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k500InternalServerError, "Failed to fetch route assignments"));
        });
}
