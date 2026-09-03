#include "TrackingWebSocketCtrl.h"
#include "../services/TrackingService.h"
#include "../utils/JwtUtils.h"
#include "../utils/Roles.h"
#include "../utils/ValidationUtils.h"

#include <json/json.h>

using namespace drogon;

namespace
{
void sendError(const WebSocketConnectionPtr& conn, const std::string& message)
{
    Json::Value j;
    j["error"] = message;
    conn->send(j.toStyledString());
}
}  

void TrackingWebSocketCtrl::handleNewConnection(const HttpRequestPtr& req,
                                                 const WebSocketConnectionPtr& conn)
{
    
    
    std::string token = req->getParameter("token");
    auto claims = JwtUtils::verifyToken(token);

    if (!claims.has_value())
    {
        LOG_WARN << "Rejecting WebSocket connection: missing/invalid token";
        conn->shutdown(CloseCode::kViolation, "Missing or invalid token");
        return;
    }

    conn->setContext(std::make_shared<AuthClaims>(*claims));
}

void TrackingWebSocketCtrl::handleNewMessage(const WebSocketConnectionPtr& conn,
                                              std::string&& message,
                                              const WebSocketMessageType& type)
{
    if (type != WebSocketMessageType::Text)
    {
        return;
    }

    auto claims = conn->getContext<AuthClaims>();
    if (!claims)
    {
        sendError(conn, "Not authenticated");
        conn->shutdown();
        return;
    }

    Json::Value msg;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream iss(message);
    if (!Json::parseFromStream(builder, iss, &msg, &errs) || !msg["action"].isString())
    {
        sendError(conn, "Message must be JSON with a string 'action' field");
        return;
    }

    std::string action = msg["action"].asString();

    if (action == "subscribe")
    {
        if (!msg["bus_id"].isString() || !ValidationUtils::isValidUuid(msg["bus_id"].asString()))
        {
            sendError(conn, "subscribe requires a valid bus_id");
            return;
        }
        TrackingService::subscribe(conn, msg["bus_id"].asString());

        Json::Value ack;
        ack["message"] = "subscribed";
        ack["bus_id"] = msg["bus_id"].asString();
        conn->send(ack.toStyledString());
        return;
    }

    if (action == "push_location")
    {
        if (claims->roleId != Roles::DRIVER && claims->roleId != Roles::CONDUCTOR &&
            claims->roleId != Roles::ADMIN)
        {
            sendError(conn, "push_location requires a driver, conductor, or administrator account");
            return;
        }

        if (!msg["bus_id"].isString() || !msg["latitude"].isNumeric() || !msg["longitude"].isNumeric())
        {
            sendError(conn, "push_location requires bus_id, latitude, longitude");
            return;
        }

        std::optional<double> speed;
        if (msg["speed_kmh"].isNumeric())
        {
            speed = msg["speed_kmh"].asDouble();
        }

        std::string busId = msg["bus_id"].asString();
        TrackingService::recordLocation(busId, msg["latitude"].asDouble(), msg["longitude"].asDouble(),
            speed,
            [conn](bool ok, const Json::Value& payload) {
                if (conn->connected())
                {
                    Json::Value ack = payload;
                    ack["ok"] = ok;
                    conn->send(ack.toStyledString());
                }
            });
        return;
    }

    sendError(conn, "Unknown action; expected 'subscribe' or 'push_location'");
}

void TrackingWebSocketCtrl::handleConnectionClosed(const WebSocketConnectionPtr& conn)
{
    TrackingService::unsubscribe(conn);
}
