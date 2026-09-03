#include "TrackingService.h"

#include "../repositories/TrackingRepo.h"
#include "../utils/GeoUtils.h"
#include "../utils/ValidationUtils.h"

#include <drogon/drogon.h>
#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

using namespace drogon;

namespace
{
Json::Value locationToJson(const TrackingRepo::LocationRow& loc)
{
    Json::Value j;
    j["bus_id"] = loc.getValueOfBusId();
    j["latitude"] = std::stod(loc.getValueOfLatitude());
    j["longitude"] = std::stod(loc.getValueOfLongitude());
    if (loc.getSpeed())
    {
        j["speed_kmh"] = std::stod(loc.getValueOfSpeed());
    }
    else
    {
        
        
        
        
        
        j["speed_kmh"] = Json::Value();
    }
    j["timestamp"] = loc.getValueOfTimestamp().toDbStringLocal();
    return j;
}

Json::Value errorJson(const std::string& message)
{
    Json::Value j;
    j["error"] = message;
    return j;
}

HttpResponsePtr jsonError(HttpStatusCode code, const std::string& message)
{
    auto resp = HttpResponse::newHttpJsonResponse(errorJson(message));
    resp->setStatusCode(code);
    return resp;
}







constexpr double kMaxPlausibleSpeedKmh = 300.0;





std::mutex g_subsMutex;
std::unordered_map<std::string, std::unordered_set<WebSocketConnectionPtr>> g_busSubscribers;
std::unordered_map<WebSocketConnectionPtr, std::string> g_connToBus;
}  

void TrackingService::recordLocation(const std::string& busId,
                                      double latitude,
                                      double longitude,
                                      std::optional<double> speedKmh,
                                      JsonResultCallback callback)
{
    if (!ValidationUtils::isValidUuid(busId))
    {
        callback(false, errorJson("bus_id must be a valid UUID"));
        return;
    }

    if (!ValidationUtils::isValidLatitude(latitude) || !ValidationUtils::isValidLongitude(longitude))
    {
        callback(false, errorJson("latitude must be in [-90, 90] and longitude in [-180, 180]"));
        return;
    }

    
    
    
    if (speedKmh.has_value() && (*speedKmh < 0 || *speedKmh > kMaxPlausibleSpeedKmh))
    {
        callback(false, errorJson("speed_kmh must be between 0 and 300"));
        return;
    }

    auto persist = [busId, latitude, longitude, callback](std::optional<double> computedSpeed) {
        
        
        
        if (computedSpeed.has_value() && (*computedSpeed < 0 || *computedSpeed > kMaxPlausibleSpeedKmh))
        {
            computedSpeed.reset();
        }

        TrackingRepo::insertLocation(busId, latitude, longitude, computedSpeed,
            [callback](const TrackingRepo::LocationRow& inserted) {
                Json::Value payload = locationToJson(inserted);

                
                std::vector<WebSocketConnectionPtr> targets;
                {
                    std::lock_guard<std::mutex> lock(g_subsMutex);
                    auto it = g_busSubscribers.find(payload["bus_id"].asString());
                    if (it != g_busSubscribers.end())
                    {
                        targets.assign(it->second.begin(), it->second.end());
                    }
                }
                std::string message = payload.toStyledString();
                for (auto& conn : targets)
                {
                    if (conn && conn->connected())
                    {
                        conn->send(message);
                    }
                }

                callback(true, payload);
            },
            [callback](const std::string& err) {
                callback(false, errorJson(err));
            });
    };

    if (speedKmh.has_value())
    {
        persist(speedKmh);
        return;
    }

    
    TrackingRepo::getLatestLocation(busId,
        [latitude, longitude, persist](std::optional<TrackingRepo::LocationRow> prev) {
            if (!prev.has_value())
            {
                persist(std::nullopt);
                return;
            }

            double prevLat = std::stod(prev->getValueOfLatitude());
            double prevLon = std::stod(prev->getValueOfLongitude());
            double seconds = (trantor::Date::now().secondsSinceEpoch() -
                               prev->getValueOfTimestamp().secondsSinceEpoch());

            if (seconds <= 0)
            {
                persist(std::nullopt);
                return;
            }

            double speed = GeoUtils::speedKmh(prevLat, prevLon, latitude, longitude, seconds);
            persist(speed);
        },
        [persist](const std::string&) {
            
            persist(std::nullopt);
        });
}

void TrackingService::getLatestLocation(const std::string& busId, ResponseCallback callback)
{
    if (!ValidationUtils::isValidUuid(busId))
    {
        callback(jsonError(k400BadRequest, "bus_id must be a valid UUID"));
        return;
    }

    TrackingRepo::getLatestLocation(busId,
        [callback](std::optional<TrackingRepo::LocationRow> loc) {
            if (!loc.has_value())
            {
                callback(jsonError(k404NotFound, "No location has been recorded for this bus yet"));
                return;
            }
            callback(HttpResponse::newHttpJsonResponse(locationToJson(*loc)));
        },
        [callback](const std::string& err) {
            callback(jsonError(k500InternalServerError, err));
        });
}

void TrackingService::getLocationHistory(const std::string& busId, int limit, ResponseCallback callback)
{
    if (!ValidationUtils::isValidUuid(busId))
    {
        callback(jsonError(k400BadRequest, "bus_id must be a valid UUID"));
        return;
    }

    
    
    int clampedLimit = (std::min)((std::max)(limit, 1), 500);

    TrackingRepo::getLocationHistory(busId, clampedLimit,
        [callback](const std::vector<TrackingRepo::LocationRow>& rows) {
            Json::Value ret(Json::arrayValue);
            for (const auto& row : rows)
            {
                ret.append(locationToJson(row));
            }
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const std::string& err) {
            callback(jsonError(k500InternalServerError, err));
        });
}

void TrackingService::subscribe(const WebSocketConnectionPtr& conn, const std::string& busId)
{
    std::lock_guard<std::mutex> lock(g_subsMutex);

    
    auto existing = g_connToBus.find(conn);
    if (existing != g_connToBus.end())
    {
        g_busSubscribers[existing->second].erase(conn);
    }

    g_busSubscribers[busId].insert(conn);
    g_connToBus[conn] = busId;
}

void TrackingService::unsubscribe(const WebSocketConnectionPtr& conn)
{
    std::lock_guard<std::mutex> lock(g_subsMutex);

    auto it = g_connToBus.find(conn);
    if (it != g_connToBus.end())
    {
        g_busSubscribers[it->second].erase(conn);
        g_connToBus.erase(it);
    }
}