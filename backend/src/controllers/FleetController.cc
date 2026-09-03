#include "FleetController.h"
#include "../models/Buses.h"

using namespace drogon;
using namespace drogon_model::postgres;

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





void FleetController::createBus(const HttpRequestPtr& req, 
                                std::function<void(const HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json)["license_plate"].isString() || !(*json)["capacity"].isInt()) {
        callback(jsonError(k400BadRequest, "Missing license_plate or capacity"));
        return;
    }

    std::string licensePlate = (*json)["license_plate"].asString();
    int capacity = (*json)["capacity"].asInt();

    if (licensePlate.empty() || licensePlate.size() > 20) {
        callback(jsonError(k400BadRequest, "license_plate must be between 1 and 20 characters"));
        return;
    }

    if (capacity <= 0 || capacity > 200) {
        callback(jsonError(k400BadRequest, "capacity must be between 1 and 200"));
        return;
    }

    
    Buses newBus;
    newBus.setLicensePlate(licensePlate);
    newBus.setCapacity(capacity);
    newBus.setStatus("ACTIVE"); 

    
    auto dbClient = app().getDbClient();
    orm::Mapper<Buses> mapper(dbClient);

    mapper.insert(newBus,
        [callback](const Buses& b) {
            Json::Value ret;
            ret["message"] = "Bus created successfully";
            ret["bus_id"] = b.getValueOfId();
            ret["license_plate"] = b.getValueOfLicensePlate();
            
            auto resp = HttpResponse::newHttpJsonResponse(ret);
            resp->setStatusCode(k201Created);
            callback(resp);
        },
        [callback](const orm::DrogonDbException& e) {
            callback(jsonError(k409Conflict, "A bus with that license plate already exists"));
        });
}




void FleetController::getAllBuses(const HttpRequestPtr& req, 
                                  std::function<void(const HttpResponsePtr&)>&& callback) {
    
    auto dbClient = app().getDbClient();
    orm::Mapper<Buses> mapper(dbClient);

    
    mapper.findAll(
        [callback](const std::vector<Buses>& buses) {
            Json::Value ret(Json::arrayValue); 

            for (const auto& bus : buses) {
                Json::Value busJson;
                busJson["id"] = bus.getValueOfId();
                busJson["license_plate"] = bus.getValueOfLicensePlate();
                busJson["capacity"] = bus.getValueOfCapacity();
                
                
                busJson["status"] = bus.getValueOfStatus(); 
                
                ret.append(busJson);
            }

            auto resp = HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        },
        [callback](const orm::DrogonDbException& e) {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k500InternalServerError);
            resp->setBody("{\"error\": \"Failed to fetch buses\"}");
            callback(resp);
        });
}