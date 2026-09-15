#pragma once

#include <drogon/HttpController.h>

class FleetController : public drogon::HttpController<FleetController> {
public:
    METHOD_LIST_BEGIN
        
        ADD_METHOD_TO(FleetController::createBus, "/api/v1/fleet/buses", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(FleetController::getAllBuses, "/api/v1/fleet/buses", drogon::Get, "JwtAuthFilter");
        ADD_METHOD_TO(FleetController::submitFeedback, "/api/v1/fleet/buses/{1}/feedback", drogon::Post);
        ADD_METHOD_TO(FleetController::getFeedback, "/api/v1/fleet/buses/{1}/feedback", drogon::Get);
    METHOD_LIST_END

    void createBus(const drogon::HttpRequestPtr& req, 
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getAllBuses(const drogon::HttpRequestPtr& req, 
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void submitFeedback(const drogon::HttpRequestPtr& req, 
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                        std::string busId);

    void getFeedback(const drogon::HttpRequestPtr& req, 
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                     std::string busId);
};