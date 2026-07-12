#pragma once

#include <drogon/HttpResponse.h>
#include <drogon/WebSocketConnection.h>
#include <functional>
#include <optional>
#include <string>
#include <json/json.h>

// TrackingService is the single place that knows how bus location updates
// are validated, persisted, turned into a speed estimate, and fanned out
// to any WebSocket clients subscribed to that bus. Both the HTTP
// TrackingController and the TrackingWebSocketCtrl call into this so the
// two transports share identical behavior.
class TrackingService
{
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;
    using JsonResultCallback = std::function<void(bool ok, const Json::Value& payloadOrError)>;

    // Validates and records a new location reading for a bus. If `speedKmh`
    // isn't supplied, it's derived from the distance/time versus the bus's
    // previous reading (if one exists). On success, broadcasts the update
    // to any subscribed WebSocket clients.
    static void recordLocation(const std::string& busId,
                                double latitude,
                                double longitude,
                                std::optional<double> speedKmh,
                                JsonResultCallback callback);

    static void getLatestLocation(const std::string& busId, ResponseCallback callback);
    static void getLocationHistory(const std::string& busId, int limit, ResponseCallback callback);

    // WebSocket subscription management. A connection subscribes to a
    // single bus_id at a time (subscribing again just changes it).
    static void subscribe(const drogon::WebSocketConnectionPtr& conn, const std::string& busId);
    static void unsubscribe(const drogon::WebSocketConnectionPtr& conn);
};
