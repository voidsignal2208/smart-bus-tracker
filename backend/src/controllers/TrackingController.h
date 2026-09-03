#pragma once

#include <drogon/HttpController.h>




class TrackingController : public drogon::HttpController<TrackingController>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(TrackingController::postLocation,
                      "/api/v1/tracking/buses/{1}/location", drogon::Post,
                      "JwtAuthFilter", "StaffOnlyFilter");
        ADD_METHOD_TO(TrackingController::getLatestLocation,
                      "/api/v1/tracking/buses/{1}/location", drogon::Get,
                      "JwtAuthFilter");
        ADD_METHOD_TO(TrackingController::getHistory,
                      "/api/v1/tracking/buses/{1}/history", drogon::Get,
                      "JwtAuthFilter");
    METHOD_LIST_END

    void postLocation(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                       std::string busId);

    void getLatestLocation(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                            std::string busId);

    void getHistory(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                     std::string busId);
};
