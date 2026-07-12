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
        // A default-constructed Json::Value is null-typed — behaviorally
        // identical to the deprecated Json::Value::null static member,
        // but doesn't depend on that symbol being exported from jsoncpp's
        // library (it isn't reliably exported from newer/vcpkg builds,
        // which causes an unresolved external symbol on MSVC).
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

// The DB column is DECIMAL(5,2), i.e. max magnitude 999.99. Reject
// obviously-bad speed values instead of letting the insert fail with a
// confusing DB error. Declared at namespace scope (not as a local inside
// recordLocation) so the lambdas below can use it without capturing it —
// MSVC, unlike GCC/Clang, requires an explicit capture for a local
// constexpr variable that's odr-used inside a nested lambda.
constexpr double kMaxPlausibleSpeedKmh = 300.0;

// --- WebSocket subscriber registry -----------------------------------
// Guarded by a mutex since Drogon may dispatch WebSocket callbacks from
// different IO-loop threads than the one handling an HTTP-triggered
// location update.
std::mutex g_subsMutex;
std::unordered_map<std::string, std::unordered_set<WebSocketConnectionPtr>> g_busSubscribers;
std::unordered_map<WebSocketConnectionPtr, std::string> g_connToBus;
}  // namespace

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

    // The DB column is DECIMAL(5,2), i.e. max magnitude 999.99. Reject
    // obviously-bad client-supplied values instead of letting the insert
    // fail with a confusing DB error.
    if (speedKmh.has_value() && (*speedKmh < 0 || *speedKmh > kMaxPlausibleSpeedKmh))
    {
        callback(false, errorJson("speed_kmh must be between 0 and 300"));
        return;
    }

    auto persist = [busId, latitude, longitude, callback](std::optional<double> computedSpeed) {
        // A derived speed that's wildly implausible (e.g. from a GPS jump,
        // or from clock skew between readings) shouldn't block recording
        // the position — just store the point without a speed instead.
        if (computedSpeed.has_value() && (*computedSpeed < 0 || *computedSpeed > kMaxPlausibleSpeedKmh))
        {
            computedSpeed.reset();
        }

        TrackingRepo::insertLocation(busId, latitude, longitude, computedSpeed,
            [callback](const TrackingRepo::LocationRow& inserted) {
                Json::Value payload = locationToJson(inserted);

                // Broadcast to subscribed WebSocket clients.
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

    // No speed supplied: derive it from the previous reading, if any.
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
            // If we can't look up history, still record the point without a speed.
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

    // Parenthesized to prevent expansion by Windows.h's min/max macros,
    // which would otherwise mis-parse std::min/std::max on MSVC.
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

    // If already subscribed to a different bus, remove that subscription first.
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