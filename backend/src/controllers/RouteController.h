#pragma once

#include <drogon/HttpController.h>

class RouteController : public drogon::HttpController<RouteController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(RouteController::createRoute, "/api/v1/routes", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::getAllRoutes, "/api/v1/routes", drogon::Get);
        ADD_METHOD_TO(RouteController::updateRoute, "/api/v1/routes/{1}", drogon::Put, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::deleteRoute, "/api/v1/routes/{1}", drogon::Delete, "JwtAuthFilter", "AdminOnlyFilter");

        ADD_METHOD_TO(RouteController::addStop, "/api/v1/routes/{1}/stops", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::getRouteStops, "/api/v1/routes/{1}/stops", drogon::Get);
        ADD_METHOD_TO(RouteController::updateStop, "/api/v1/routes/{1}/stops/{2}", drogon::Put, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::deleteStop, "/api/v1/routes/{1}/stops/{2}", drogon::Delete, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::reorderStops, "/api/v1/routes/{1}/stops/reorder", drogon::Put, "JwtAuthFilter", "AdminOnlyFilter");

        ADD_METHOD_TO(RouteController::assignBus, "/api/v1/routes/{1}/buses", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::getRouteBuses, "/api/v1/routes/{1}/buses", drogon::Get);
    METHOD_LIST_END

    void createRoute(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getAllRoutes(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void updateRoute(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                     std::string routeId);

    void deleteRoute(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                     std::string routeId);

    void addStop(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                 std::string routeId);

    void getRouteStops(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       std::string routeId);

    void updateStop(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    std::string routeId,
                    std::string stopId);

    void deleteStop(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                    std::string routeId,
                    std::string stopId);

    void reorderStops(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                      std::string routeId);

    void assignBus(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                   std::string routeId);

    void getRouteBuses(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       std::string routeId);
};
