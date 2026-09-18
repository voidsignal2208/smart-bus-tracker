#include "MapsController.h"
#include "../utils/EnvConfig.h"
#include "../utils/ValidationUtils.h"

#include <drogon/drogon.h>
#include <drogon/HttpClient.h>

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

const std::string& directionsApiKey()
{
    static const std::string key = EnvConfig::require("GOOGLE_MAPS_SERVER_KEY");
    return key;
}

// Google's Directions API returns an encoded polyline string (a compact
// text encoding of a list of lat/lng points) rather than raw coordinates.
// We pass that string straight through to the frontend, which decodes it
// with the Google Maps JS library's own google.maps.geometry.encoding
// helper - no need to decode it server-side at all.
//for the frontend guy to do
}  // namespace

void MapsController::getRoutePolyline(const HttpRequestPtr& req,
                                       std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto originLatStr = req->getParameter("origin_lat");
    auto originLngStr = req->getParameter("origin_lng");
    auto destLatStr = req->getParameter("dest_lat");
    auto destLngStr = req->getParameter("dest_lng");

    if (originLatStr.empty() || originLngStr.empty() || destLatStr.empty() || destLngStr.empty())
    {
        callback(jsonError(k400BadRequest,
                            "origin_lat, origin_lng, dest_lat, dest_lng are all required"));
        return;
    }

    double originLat, originLng, destLat, destLng;
    try
    {
        originLat = std::stod(originLatStr);
        originLng = std::stod(originLngStr);
        destLat = std::stod(destLatStr);
        destLng = std::stod(destLngStr);
    }
    catch (...)
    {
        callback(jsonError(k400BadRequest, "Coordinates must be valid numbers"));
        return;
    }

    if (!ValidationUtils::isValidLatitude(originLat) || !ValidationUtils::isValidLatitude(destLat) ||
        !ValidationUtils::isValidLongitude(originLng) || !ValidationUtils::isValidLongitude(destLng))
    {
        callback(jsonError(k400BadRequest, "One or more coordinates are out of range"));
        return;
    }

    auto client = HttpClient::newHttpClient("https://maps.googleapis.com");

    std::string path = "/maps/api/directions/json"
                        "?origin=" + originLatStr + "," + originLngStr +
                        "&destination=" + destLatStr + "," + destLngStr +
                        "&mode=driving"
                        "&key=" + directionsApiKey();

    auto googleReq = HttpRequest::newHttpRequest();
    googleReq->setMethod(Get);
    googleReq->setPath(path);

    client->sendRequest(googleReq,
        [callback](ReqResult result, const HttpResponsePtr& googleResp) {
            if (result != ReqResult::Ok || !googleResp)
            {
                callback(jsonError(k502BadGateway, "Could not reach Google Directions API"));
                return;
            }

            auto googleJson = googleResp->getJsonObject();
            if (!googleJson || (*googleJson)["status"].asString() != "OK")
            {
                std::string status = googleJson ? (*googleJson)["status"].asString() : "UNKNOWN";
                callback(jsonError(k502BadGateway, "Directions API returned status: " + status));
                return;
            }

            const auto& route = (*googleJson)["routes"][0];
            const auto& leg = route["legs"][0];

            Json::Value ret;
            ret["polyline"] = route["overview_polyline"]["points"];
            ret["distance_meters"] = leg["distance"]["value"];
            ret["duration_seconds"] = leg["duration"]["value"];

            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        30 /* seconds timeout */);
}