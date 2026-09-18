#include "FleetController.h"
#include "../models/Buses.h"
#include "../utils/ValidationUtils.h"
#include "../utils/JwtUtils.h"

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
}  // namespace

// ----------------------------------------------------------------------
// API: POST /api/v1/fleet/buses
// Body: { "license_plate": "XYZ-1234", "capacity": 50, "route_id": "uuid", "driver_id": "uuid" }
// ----------------------------------------------------------------------
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

    auto dbClient = app().getDbClient();
    const std::string routeId = (*json)["route_id"].isString() ? (*json)["route_id"].asString() : "";
    const std::string driverId = (*json)["driver_id"].isString() ? (*json)["driver_id"].asString() : "";
    if ((!routeId.empty() && !ValidationUtils::isValidUuid(routeId)) ||
        (!driverId.empty() && !ValidationUtils::isValidUuid(driverId))) {
        callback(jsonError(k400BadRequest, "route_id and driver_id must be valid UUIDs"));
        return;
    }

    dbClient->execSqlAsync(
        "INSERT INTO buses (license_plate, capacity, status) VALUES ($1, $2, 'ACTIVE') RETURNING id, license_plate",
        [dbClient, routeId, driverId, callback](const orm::Result& r) {
            const std::string busId = r[0]["id"].as<std::string>();
            if (routeId.empty() && driverId.empty()) {
                Json::Value ret;
                ret["message"] = "Bus created successfully";
                ret["bus_id"] = busId;
                ret["license_plate"] = r[0]["license_plate"].as<std::string>();
                auto resp = HttpResponse::newHttpJsonResponse(ret);
                resp->setStatusCode(k201Created);
                callback(resp);
                return;
            }
            dbClient->execSqlAsync(
                "INSERT INTO bus_assignments (bus_id, route_id, driver_id, status, assignment_date) "
                "VALUES ($1::uuid, NULLIF($2, '')::uuid, NULLIF($3, '')::uuid, 'SCHEDULED', CURRENT_DATE)",
                [callback, busId, licensePlate = r[0]["license_plate"].as<std::string>()](const orm::Result&) {
                    Json::Value ret;
                    ret["message"] = "Bus created successfully";
                    ret["bus_id"] = busId;
                    ret["license_plate"] = licensePlate;
                    auto resp = HttpResponse::newHttpJsonResponse(ret);
                    resp->setStatusCode(k201Created);
                    callback(resp);
                },
                [callback](const orm::DrogonDbException& e) {
                    LOG_ERROR << "Failed to assign new bus: " << e.base().what();
                    callback(jsonError(k400BadRequest, "Bus created, but its route or driver assignment is invalid"));
                },
                busId, routeId, driverId);
        },
        [callback](const orm::DrogonDbException&) {
            callback(jsonError(k409Conflict, "A bus with that license plate already exists"));
        },
        licensePlate, capacity);
}

// ----------------------------------------------------------------------
// API: GET /api/v1/fleet/buses
// ----------------------------------------------------------------------
void FleetController::getAllBuses(const HttpRequestPtr& req, 
                                  std::function<void(const HttpResponsePtr&)>&& callback) {
    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "SELECT b.id, b.license_plate, b.capacity, b.status, COALESCE(b.rating, 0) as rating, COALESCE(b.feedback_score, 0) as feedback_score, "
        "a.route_id::text AS route_id, a.driver_id::text AS driver_id, r.name AS route_name, u.name AS driver_name "
        "FROM buses b LEFT JOIN LATERAL (SELECT * FROM bus_assignments WHERE bus_id = b.id ORDER BY assignment_date DESC, created_at DESC LIMIT 1) a ON true "
        "LEFT JOIN routes r ON r.id = a.route_id LEFT JOIN users u ON u.id = a.driver_id ORDER BY b.license_plate ASC",
        [callback](const drogon::orm::Result& r) {
            Json::Value ret(Json::arrayValue);

            for (const auto& row : r) {
                Json::Value busJson;
                busJson["id"] = row["id"].as<std::string>();
                busJson["license_plate"] = row["license_plate"].as<std::string>();
                busJson["capacity"] = row["capacity"].as<int>();
                busJson["status"] = row["status"].as<std::string>();
                busJson["rating"] = std::stod(row["rating"].as<std::string>());
                busJson["feedback_score"] = std::stod(row["feedback_score"].as<std::string>());
                if (!row["route_id"].isNull()) busJson["route_id"] = row["route_id"].as<std::string>();
                if (!row["driver_id"].isNull()) busJson["driver_id"] = row["driver_id"].as<std::string>();
                if (!row["route_name"].isNull()) busJson["route_name"] = row["route_name"].as<std::string>();
                if (!row["driver_name"].isNull()) busJson["driver_name"] = row["driver_name"].as<std::string>();
                ret.append(busJson);
            }

            auto resp = HttpResponse::newHttpJsonResponse(ret);
            callback(resp);
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to fetch buses: " << e.base().what();
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k500InternalServerError);
            resp->setBody("{\"error\": \"Failed to fetch buses\"}");
            callback(resp);
        });
}

// ----------------------------------------------------------------------
// API: GET /api/v1/fleet/drivers (administrator only)
// ----------------------------------------------------------------------
void FleetController::getDrivers(const HttpRequestPtr& req,
                                 std::function<void(const HttpResponsePtr&)>&& callback) {
    app().getDbClient()->execSqlAsync(
        "SELECT id, name, email FROM users WHERE role_id = 2 ORDER BY name ASC",
        [callback](const orm::Result& r) {
            Json::Value ret(Json::arrayValue);
            for (const auto& row : r) {
                Json::Value driver;
                driver["id"] = row["id"].as<std::string>();
                driver["name"] = row["name"].as<std::string>();
                driver["email"] = row["email"].as<std::string>();
                ret.append(driver);
            }
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to fetch drivers: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to fetch drivers"));
        });
}

// ----------------------------------------------------------------------
// API: GET /api/v1/fleet/my-assignment (driver's own current assignment)
// ----------------------------------------------------------------------
void FleetController::getMyAssignment(const HttpRequestPtr& req,
                                      std::function<void(const HttpResponsePtr&)>&& callback) {
    const int roleId = req->getAttributes()->get<int>("role_id");
    if (roleId != 2) {
        callback(jsonError(k403Forbidden, "This endpoint is available to drivers only"));
        return;
    }
    const std::string userId = req->getAttributes()->get<std::string>("user_id");
    app().getDbClient()->execSqlAsync(
        "SELECT b.id AS bus_id, b.license_plate, b.capacity, b.status, r.id AS route_id, r.name AS route_name, u.name AS driver_name "
        "FROM bus_assignments a JOIN buses b ON b.id = a.bus_id LEFT JOIN routes r ON r.id = a.route_id "
        "JOIN users u ON u.id = a.driver_id WHERE a.driver_id = $1::uuid "
        "ORDER BY a.assignment_date DESC, a.created_at DESC LIMIT 1",
        [callback](const orm::Result& r) {
            if (r.empty()) {
                callback(jsonError(k404NotFound, "No bus is assigned to this driver"));
                return;
            }
            Json::Value ret;
            ret["bus_id"] = r[0]["bus_id"].as<std::string>();
            ret["license_plate"] = r[0]["license_plate"].as<std::string>();
            ret["capacity"] = r[0]["capacity"].as<int>();
            ret["status"] = r[0]["status"].as<std::string>();
            ret["driver_name"] = r[0]["driver_name"].as<std::string>();
            if (!r[0]["route_id"].isNull()) ret["route_id"] = r[0]["route_id"].as<std::string>();
            if (!r[0]["route_name"].isNull()) ret["route_name"] = r[0]["route_name"].as<std::string>();
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to fetch driver assignment: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to fetch driver assignment"));
        }, userId);
}

// ----------------------------------------------------------------------
// API: PUT /api/v1/fleet/buses/{busId}
// Body: { "license_plate": "XYZ-1234", "capacity": 50, "status": "ACTIVE", "route_id": "uuid", "driver_id": "uuid" }
// ----------------------------------------------------------------------
void FleetController::updateBus(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback,
                                std::string busId) {
    if (!ValidationUtils::isValidUuid(busId)) {
        callback(jsonError(k400BadRequest, "busId must be a valid UUID"));
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !(*json)["license_plate"].isString() || !(*json)["capacity"].isInt() ||
        !(*json)["status"].isString()) {
        callback(jsonError(k400BadRequest, "license_plate, capacity, and status are required"));
        return;
    }

    const std::string licensePlate = (*json)["license_plate"].asString();
    const int capacity = (*json)["capacity"].asInt();
    const std::string status = (*json)["status"].asString();
    const std::string routeId = (*json)["route_id"].isString() ? (*json)["route_id"].asString() : "";
    const std::string driverId = (*json)["driver_id"].isString() ? (*json)["driver_id"].asString() : "";
    if (licensePlate.empty() || licensePlate.size() > 20 || capacity <= 0 || capacity > 200 || status.empty() || status.size() > 50) {
        callback(jsonError(k400BadRequest, "Invalid bus data"));
        return;
    }
    if ((!routeId.empty() && !ValidationUtils::isValidUuid(routeId)) ||
        (!driverId.empty() && !ValidationUtils::isValidUuid(driverId))) {
        callback(jsonError(k400BadRequest, "route_id and driver_id must be valid UUIDs"));
        return;
    }

    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "UPDATE buses SET license_plate = $1, capacity = $2, status = $3 WHERE id = $4::uuid RETURNING id, license_plate, capacity, status",
        [dbClient, callback, busId, routeId, driverId](const orm::Result& r) {
            if (r.empty()) {
                callback(jsonError(k404NotFound, "Bus not found"));
                return;
            }
            dbClient->execSqlAsync(
                "WITH removed AS (DELETE FROM bus_assignments WHERE bus_id = $1::uuid) "
                "INSERT INTO bus_assignments (bus_id, route_id, driver_id, status, assignment_date) "
                "SELECT $1::uuid, NULLIF($2, '')::uuid, NULLIF($3, '')::uuid, 'SCHEDULED', CURRENT_DATE "
                "WHERE NULLIF($2, '') IS NOT NULL OR NULLIF($3, '') IS NOT NULL",
                [callback](const orm::Result&) {
                    Json::Value ret;
                    ret["message"] = "Bus updated successfully";
                    callback(HttpResponse::newHttpJsonResponse(ret));
                },
                [callback](const orm::DrogonDbException& e) {
                    LOG_ERROR << "Failed to update bus assignment: " << e.base().what();
                    callback(jsonError(k400BadRequest, "Bus updated, but its route or driver assignment is invalid"));
                },
                busId, routeId, driverId);
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to update bus: " << e.base().what();
            callback(jsonError(k409Conflict, "Failed to update bus; license plate may already exist"));
        },
        licensePlate, capacity, status, busId);
}

// ----------------------------------------------------------------------
// API: DELETE /api/v1/fleet/buses/{busId}
// Removes route assignments first because they use a restrictive foreign key.
// ----------------------------------------------------------------------
void FleetController::deleteBus(const HttpRequestPtr& req,
                                std::function<void(const HttpResponsePtr&)>&& callback,
                                std::string busId) {
    if (!ValidationUtils::isValidUuid(busId)) {
        callback(jsonError(k400BadRequest, "busId must be a valid UUID"));
        return;
    }

    auto dbClient = app().getDbClient();
    dbClient->execSqlAsync(
        "WITH removed_assignments AS (DELETE FROM bus_assignments WHERE bus_id = $1::uuid) "
        "DELETE FROM buses WHERE id = $1::uuid RETURNING id",
        [callback](const orm::Result& r) {
            if (r.empty()) {
                callback(jsonError(k404NotFound, "Bus not found"));
                return;
            }
            Json::Value ret;
            ret["message"] = "Bus deleted successfully";
            callback(HttpResponse::newHttpJsonResponse(ret));
        },
        [callback](const orm::DrogonDbException& e) {
            LOG_ERROR << "Failed to delete bus: " << e.base().what();
            callback(jsonError(k500InternalServerError, "Failed to delete bus"));
        },
        busId);
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
