#include "FleetController.h"
#include "../models/Buses.h"
#include "../utils/ValidationUtils.h"

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

// ----------------------------------------------------------------------
// API: POST /api/v1/fleet/buses/{busId}/feedback
// Body: { "rating": 5, "is_crowded": false, "ac_working": true, "cleanliness": 4, "comment": "Good journey" }
// ----------------------------------------------------------------------
void FleetController::submitFeedback(const HttpRequestPtr& req,
                                     std::function<void(const HttpResponsePtr&)>&& callback,
                                     std::string busId) {
    if (!ValidationUtils::isValidUuid(busId)) {
        callback(jsonError(k400BadRequest, "busId must be a valid UUID"));
        return;
    }

    auto json = req->getJsonObject();
    if (!json) {
        callback(jsonError(k400BadRequest, "Request body is required"));
        return;
    }

    int rating = 5;
    if ((*json)["rating"].isInt()) {
        rating = (std::min)(5, (std::max)(1, (*json)["rating"].asInt()));
    }

    bool isCrowded = (*json)["is_crowded"].isBool() ? (*json)["is_crowded"].asBool() : false;
    bool acWorking = (*json)["ac_working"].isBool() ? (*json)["ac_working"].asBool() : true;
    int cleanliness = 5;
    if ((*json)["cleanliness"].isInt()) {
        cleanliness = (std::min)(5, (std::max)(1, (*json)["cleanliness"].asInt()));
    }
    std::string comment = (*json)["comment"].isString() ? (*json)["comment"].asString() : "";

    auto dbClient = app().getDbClient();

    // Cast all values to strings so Drogon sends them as text params (avoids binary
    // protocol type mismatches with Supabase session pooler for numeric/bool columns)
    std::string ratingStr = std::to_string(rating);
    std::string cleanlinessStr = std::to_string(cleanliness);
    std::string isCrowdedStr = isCrowded ? "true" : "false";
    std::string acWorkingStr = acWorking ? "true" : "false";

    // Insert feedback and rating
    dbClient->execSqlAsync(
        "INSERT INTO bus_feedback (bus_id, is_crowded, ac_working, cleanliness, comment, created_at) "
        "VALUES ($1::uuid, $2::boolean, $3::boolean, $4::integer, $5, CURRENT_TIMESTAMP)",
        [dbClient, busId, ratingStr, comment, callback](const drogon::orm::Result&) {
            dbClient->execSqlAsync(
                "INSERT INTO bus_ratings (bus_id, rating, comment, created_at) "
                "VALUES ($1::uuid, $2::integer, $3, CURRENT_TIMESTAMP)",
                [dbClient, busId, callback](const drogon::orm::Result&) {
                    // Update aggregate rating & feedback score on buses table
                    dbClient->execSqlAsync(
                        "UPDATE buses SET "
                        "rating = (SELECT ROUND(AVG(rating), 2) FROM bus_ratings WHERE bus_id = $1::uuid), "
                        "feedback_score = (SELECT ROUND(AVG(cleanliness), 2) FROM bus_feedback WHERE bus_id = $1::uuid) "
                        "WHERE id = $1::uuid RETURNING rating::text, feedback_score::text",
                        [callback](const drogon::orm::Result& r) {
                            Json::Value ret;
                            ret["message"] = "Feedback submitted successfully";
                            ret["ok"] = true;
                            if (!r.empty()) {
                                try {
                                    if (!r[0]["rating"].isNull()) ret["updated_rating"] = std::stod(r[0]["rating"].as<std::string>());
                                    if (!r[0]["feedback_score"].isNull()) ret["updated_score"] = std::stod(r[0]["feedback_score"].as<std::string>());
                                } catch (...) {}
                            }
                            auto resp = HttpResponse::newHttpJsonResponse(ret);
                            resp->setStatusCode(k201Created);
                            callback(resp);
                        },

                        [callback](const orm::DrogonDbException&) {
                            Json::Value ret;
                            ret["message"] = "Feedback recorded";
                            ret["ok"] = true;
                            auto resp = HttpResponse::newHttpJsonResponse(ret);
                            resp->setStatusCode(k201Created);
                            callback(resp);
                        },
                        busId);
                },
                [callback](const orm::DrogonDbException& e) {
                    LOG_ERROR << "Failed to insert rating: " << e.base().what();
                    callback(jsonError(k500InternalServerError, "Failed to record rating"));
                },
                busId, ratingStr, comment);
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to insert feedback: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to record feedback (bus may not exist)"));
        },
        busId, isCrowdedStr, acWorkingStr, cleanlinessStr, comment);

}

// ----------------------------------------------------------------------
// API: GET /api/v1/fleet/buses/{busId}/feedback
// ----------------------------------------------------------------------
void FleetController::getFeedback(const HttpRequestPtr& req,
                                  std::function<void(const HttpResponsePtr&)>&& callback,
                                  std::string busId) {
    if (!ValidationUtils::isValidUuid(busId)) {
        callback(jsonError(k400BadRequest, "busId must be a valid UUID"));
        return;
    }

    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT "
        "  COALESCE((SELECT ROUND(AVG(rating), 2)::text FROM bus_ratings WHERE bus_id = $1::uuid), '5.0') AS avg_rating, "
        "  COALESCE((SELECT COUNT(*)::text FROM bus_ratings WHERE bus_id = $1::uuid), '0') AS total_reviews, "
        "  COALESCE((SELECT ROUND(AVG(cleanliness), 2)::text FROM bus_feedback WHERE bus_id = $1::uuid), '5.0') AS avg_cleanliness, "
        "  COALESCE((SELECT ROUND(100.0 * COUNT(CASE WHEN is_crowded THEN 1 END) / NULLIF(COUNT(*), 0), 1)::text FROM bus_feedback WHERE bus_id = $1::uuid), '0') AS crowded_pct, "
        "  COALESCE((SELECT ROUND(100.0 * COUNT(CASE WHEN ac_working THEN 1 END) / NULLIF(COUNT(*), 0), 1)::text FROM bus_feedback WHERE bus_id = $1::uuid), '100') AS ac_working_pct",
        [dbClient, busId, callback](const drogon::orm::Result& r) {
            Json::Value ret;
            ret["average_rating"] = 5.0;
            ret["total_reviews"] = 0;
            ret["avg_cleanliness"] = 5.0;
            ret["crowded_pct"] = 0.0;
            ret["ac_working_pct"] = 100.0;

            if (!r.empty()) {
                const auto& row = r[0];
                try {
                    if (!row["avg_rating"].isNull()) ret["average_rating"] = std::stod(row["avg_rating"].as<std::string>());
                    if (!row["total_reviews"].isNull()) ret["total_reviews"] = std::stoi(row["total_reviews"].as<std::string>());
                    if (!row["avg_cleanliness"].isNull()) ret["avg_cleanliness"] = std::stod(row["avg_cleanliness"].as<std::string>());
                    if (!row["crowded_pct"].isNull()) ret["crowded_pct"] = std::stod(row["crowded_pct"].as<std::string>());
                    if (!row["ac_working_pct"].isNull()) ret["ac_working_pct"] = std::stod(row["ac_working_pct"].as<std::string>());
                } catch (...) {
                    // Fallback to default values
                }
            }

            // Get recent comments
            dbClient->execSqlAsync(
                "SELECT COALESCE(comment, '') AS comment, "
                "COALESCE(cleanliness, 5)::text AS cleanliness, "
                "COALESCE(is_crowded, false)::text AS is_crowded, "
                "COALESCE(ac_working, true)::text AS ac_working, "
                "COALESCE(created_at::text, '') AS created_at "
                "FROM bus_feedback WHERE bus_id = $1::uuid AND comment IS NOT NULL AND comment != '' "
                "ORDER BY created_at DESC LIMIT 5",
                [ret, callback](const drogon::orm::Result& comments) mutable {
                    Json::Value recent(Json::arrayValue);
                    for (const auto& c : comments) {
                        Json::Value item;
                        item["comment"] = c["comment"].as<std::string>();
                        try {
                            item["cleanliness"] = std::stoi(c["cleanliness"].as<std::string>());
                        } catch (...) {
                            item["cleanliness"] = 5;
                        }
                        std::string crowdedStr = c["is_crowded"].as<std::string>();
                        item["is_crowded"] = (crowdedStr == "t" || crowdedStr == "true" || crowdedStr == "1");
                        std::string acStr = c["ac_working"].as<std::string>();
                        item["ac_working"] = (acStr == "t" || acStr == "true" || acStr == "1");
                        item["created_at"] = c["created_at"].as<std::string>();
                        recent.append(item);
                    }
                    ret["recent_comments"] = recent;
                    callback(HttpResponse::newHttpJsonResponse(ret));
                },
                [ret, callback](const orm::DrogonDbException&) {
                    callback(HttpResponse::newHttpJsonResponse(ret));
                },
                busId);
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to fetch feedback summary: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to fetch feedback"));
        },
        busId);

}
