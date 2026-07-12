#pragma once

#include <drogon/HttpController.h>

class RouteController : public drogon::HttpController<RouteController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(RouteController::createRoute, "/api/v1/routes", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::getAllRoutes, "/api/v1/routes", drogon::Get, "JwtAuthFilter");

        ADD_METHOD_TO(RouteController::addStop, "/api/v1/routes/{1}/stops", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(RouteController::getRouteStops, "/api/v1/routes/{1}/stops", drogon::Get, "JwtAuthFilter");
    METHOD_LIST_END

    void createRoute(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getAllRoutes(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void addStop(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                 std::string routeId);

    void getRouteStops(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       std::string routeId);
};
