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
}  // namespace

// ----------------------------------------------------------------------
// API: POST /api/v1/tracking/buses/{busId}/location
// Body: { "latitude": 40.71, "longitude": -74.00, "speed_kmh": 22.5 }
// speed_kmh is optional; if omitted it's derived from the previous reading.
// Restricted to DRIVER/CONDUCTOR/ADMIN accounts.
// ----------------------------------------------------------------------
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

// ----------------------------------------------------------------------
// API: GET /api/v1/tracking/buses/{busId}/location
// ----------------------------------------------------------------------
void TrackingController::getLatestLocation(const HttpRequestPtr& req,
                                            std::function<void(const HttpResponsePtr&)>&& callback,
                                            std::string busId)
{
    TrackingService::getLatestLocation(busId, callback);
}

// ----------------------------------------------------------------------
// API: GET /api/v1/tracking/buses/{busId}/history?limit=50
// ----------------------------------------------------------------------
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
