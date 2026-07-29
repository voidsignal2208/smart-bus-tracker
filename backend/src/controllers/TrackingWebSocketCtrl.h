#pragma once

#include <drogon/WebSocketController.h>

// Realtime tracking channel at ws://.../ws/tracking?token=<JWT>
//
// Protocol (JSON text frames both ways):
//   Driver/conductor/admin client sends:
//     {"action":"push_location","bus_id":"<uuid>","latitude":..,"longitude":..,"speed_kmh":..}
//     -> server persists it and broadcasts to subscribers, replies with an ack or error.
//
//   Passenger (or any authenticated) client sends:
//     {"action":"subscribe","bus_id":"<uuid>"}
//     -> server starts pushing {"bus_id":...,"latitude":...,"longitude":...,"speed_kmh":...,"timestamp":...}
//        messages whenever that bus's location updates (from any client, WS or HTTP).
//
// Authentication happens at connection time via a `token` query parameter
// (browsers can't set custom headers during the WebSocket handshake), since
// JwtAuthFilter can't be reused as-is for that scenario.
class TrackingWebSocketCtrl : public drogon::WebSocketController<TrackingWebSocketCtrl>
{
public:
    void handleNewMessage(const drogon::WebSocketConnectionPtr& conn,
                           std::string&& message,
                           const drogon::WebSocketMessageType& type) override;

    void handleNewConnection(const drogon::HttpRequestPtr& req,
                              const drogon::WebSocketConnectionPtr& conn) override;

    void handleConnectionClosed(const drogon::WebSocketConnectionPtr& conn) override;

    WS_PATH_LIST_BEGIN
        WS_PATH_ADD("/ws/tracking");
    WS_PATH_LIST_END
};
