#include "TrackingController.h"
#include "../services/TrackingService.h"

using namespace drogon;

namespace
{
HttpResponsePtr jsonError(HttpStatusCode code, const std::string& message)
{
    Json::Value ret;
    ret["error"] = message;
    auto resp = HttpResponse::newHttpJsonResponse(ret);
    resp->setStatusCode(code);
    return resp;
}
}  







void TrackingController::postLocation(const HttpRequestPtr& req,
                                       std::function<void(const HttpResponsePtr&)>&& callback,
                                       std::string busId)
{
    auto json = req->getJsonObject();
    if (!json || !(*json)["latitude"].isNumeric() || !(*json)["longitude"].isNumeric())
    {
        callback(jsonError(k400BadRequest, "latitude and longitude are required numeric fields"));
        return;
    }

    std::optional<double> speed;
    if ((*json)["speed_kmh"].isNumeric())
    {
        speed = (*json)["speed_kmh"].asDouble();
    }

    TrackingService::recordLocation(busId,
        (*json)["latitude"].asDouble(),
        (*json)["longitude"].asDouble(),
        speed,
        [callback](bool ok, const Json::Value& payload) {
            auto resp = HttpResponse::newHttpJsonResponse(payload);
            resp->setStatusCode(ok ? k201Created : k400BadRequest);
            callback(resp);
        });
}




void TrackingController::getLatestLocation(const HttpRequestPtr& req,
                                            std::function<void(const HttpResponsePtr&)>&& callback,
                                            std::string busId)
{
    TrackingService::getLatestLocation(busId, callback);
}




void TrackingController::getHistory(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback,
                                     std::string busId)
{
    int limit = 50;
    auto limitParam = req->getParameter("limit");
    if (!limitParam.empty())
    {
        try { limit = std::stoi(limitParam); } catch (...) { limit = 50; }
    }

    TrackingService::getLocationHistory(busId, limit, callback);
}
