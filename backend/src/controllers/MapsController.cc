#include "MapsController.h"
#include "../utils/EnvConfig.h"
#include "../utils/ValidationUtils.h"

#include <drogon/drogon.h>
#include <drogon/HttpClient.h>

#include <sstream>
#include <string>
#include <vector>

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

// Parses a "lat1,lng1|lat2,lng2|..." waypoints string (route stops, in
// order) into validated "lat,lng" pairs suitable for Google's Directions
// `waypoints` param. Returns false if any pair is malformed or out of
// range - the caller treats that as a 400.
bool parseWaypoints(const std::string& raw, std::vector<std::string>& outPairs)
{
    std::stringstream ss(raw);
    std::string pair;
    while (std::getline(ss, pair, '|'))
    {
        if (pair.empty()) continue;

        auto commaPos = pair.find(',');
        if (commaPos == std::string::npos) return false;

        try
        {
            double lat = std::stod(pair.substr(0, commaPos));
            double lng = std::stod(pair.substr(commaPos + 1));
            if (!ValidationUtils::isValidLatitude(lat) || !ValidationUtils::isValidLongitude(lng))
            {
                return false;
            }
        }
        catch (...)
        {
            return false;
        }

        outPairs.push_back(pair);
    }
    return true;
}
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

    // Optional: intermediate stops the road route must pass through, in
    // order - e.g. every stop on a bus route between its origin and
    // destination. Format: "lat1,lng1|lat2,lng2|...". These are never
    // reordered (we don't ask Google to optimize:true them), since doing
    // so could put stops out of the bus's actual sequence.
    std::vector<std::string> waypointPairs;
    auto waypointsParam = req->getParameter("waypoints");
    if (!waypointsParam.empty() && !parseWaypoints(waypointsParam, waypointPairs))
    {
        callback(jsonError(k400BadRequest,
                            "waypoints must be \"lat,lng|lat,lng|...\" with valid coordinates"));
        return;
    }

    auto client = HttpClient::newHttpClient("https://maps.googleapis.com");

    std::string path = "/maps/api/directions/json"
                        "?origin=" + originLatStr + "," + originLngStr +
                        "&destination=" + destLatStr + "," + destLngStr +
                        "&mode=driving";

    if (!waypointPairs.empty())
    {
        std::string joined;
        for (size_t i = 0; i < waypointPairs.size(); ++i)
        {
            if (i > 0) joined += "|";
            joined += waypointPairs[i];
        }
        path += "&waypoints=" + joined;
    }

    path += "&key=" + directionsApiKey();

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
            const auto& legs = route["legs"];

            // With waypoints, Directions returns one leg per hop
            // (origin->stop1, stop1->stop2, ..., stopN->destination) -
            // sum them for the trip totals instead of only reading leg[0].
            unsigned long long legDistance = 0, legDuration = 0;
            for (const auto& leg : legs)
            {
                legDistance += leg["distance"]["value"].asUInt();
                legDuration += leg["duration"]["value"].asUInt();
            }

            Json::Value ret;
            ret["polyline"] = route["overview_polyline"]["points"];
            ret["distance_meters"] = static_cast<Json::UInt64>(legDistance);
            ret["duration_seconds"] = static_cast<Json::UInt64>(legDuration);

            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        30 /* seconds timeout */);
}