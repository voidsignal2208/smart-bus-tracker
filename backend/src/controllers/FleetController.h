#pragma once

#include <drogon/HttpController.h>

class FleetController : public drogon::HttpController<FleetController> {
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(FleetController::createBus, "/api/v1/fleet/buses", drogon::Post, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(FleetController::getAllBuses, "/api/v1/fleet/buses", drogon::Get);
        ADD_METHOD_TO(FleetController::getDrivers, "/api/v1/fleet/drivers", drogon::Get, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(FleetController::getMyAssignment, "/api/v1/fleet/my-assignment", drogon::Get, "JwtAuthFilter");
        ADD_METHOD_TO(FleetController::updateBus, "/api/v1/fleet/buses/{1}", drogon::Put, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(FleetController::deleteBus, "/api/v1/fleet/buses/{1}", drogon::Delete, "JwtAuthFilter", "AdminOnlyFilter");
        ADD_METHOD_TO(FleetController::submitFeedback, "/api/v1/fleet/buses/{1}/feedback", drogon::Post);
        ADD_METHOD_TO(FleetController::getFeedback, "/api/v1/fleet/buses/{1}/feedback", drogon::Get);
    METHOD_LIST_END

    void createBus(const drogon::HttpRequestPtr& req, 
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getAllBuses(const drogon::HttpRequestPtr& req, 
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getDrivers(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void getMyAssignment(const drogon::HttpRequestPtr& req,
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void updateBus(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                   std::string busId);

    void deleteBus(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                   std::string busId);

    void submitFeedback(const drogon::HttpRequestPtr& req, 
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                        std::string busId);

    void getFeedback(const drogon::HttpRequestPtr& req, 
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                     std::string busId);
};
