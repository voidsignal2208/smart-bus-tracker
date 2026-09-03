#pragma once

#include <drogon/HttpResponse.h>
#include <drogon/WebSocketConnection.h>
#include <functional>
#include <optional>
#include <string>
#include <json/json.h>






class TrackingService
{
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;
    using JsonResultCallback = std::function<void(bool ok, const Json::Value& payloadOrError)>;

    
    
    
    
    static void recordLocation(const std::string& busId,
                                double latitude,
                                double longitude,
                                std::optional<double> speedKmh,
                                JsonResultCallback callback);

    static void getLatestLocation(const std::string& busId, ResponseCallback callback);
    static void getLocationHistory(const std::string& busId, int limit, ResponseCallback callback);

    
    
    static void subscribe(const drogon::WebSocketConnectionPtr& conn, const std::string& busId);
    static void unsubscribe(const drogon::WebSocketConnectionPtr& conn);
};
