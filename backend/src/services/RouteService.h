#pragma once

#include <drogon/HttpResponse.h>
#include <functional>
#include <string>

class RouteService
{
public:
    using ResponseCallback = std::function<void(const drogon::HttpResponsePtr&)>;

    static void createRoute(const std::string& name,
                             const std::string& origin,
                             const std::string& destination,
                             ResponseCallback callback);

    static void getAllRoutes(ResponseCallback callback);

    // routeId must already be validated as a well-formed UUID by the caller.
    static void addStop(const std::string& routeId,
                         const std::string& name,
                         double latitude,
                         double longitude,
                         int sequenceOrder,
                         ResponseCallback callback);

    static void getRouteStops(const std::string& routeId, ResponseCallback callback);
};
