#pragma once

#include <drogon/HttpController.h>

class FleetController : public drogon::HttpController<FleetController> {
public:
    METHOD_LIST_BEGIN
        
        ADD_METHOD_TO(FleetController::createBus, "/api/v1/fleet/buses", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(FleetController::getAllBuses, "/api/v1/fleet/buses", drogon::Get, "JwtAuthFilter");
    METHOD_LIST_END

    void createBus(const drogon::HttpRequestPtr& req, 
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getAllBuses(const drogon::HttpRequestPtr& req, 
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};