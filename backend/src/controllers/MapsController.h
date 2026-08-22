#pragma once

#include <drogon/HttpController.h>


class MapsController : public drogon::HttpController<MapsController>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(MapsController::getRoutePolyline,
                      "/api/v1/maps/route", drogon::Get, "JwtAuthFilter");
    METHOD_LIST_END

    void getRoutePolyline(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};