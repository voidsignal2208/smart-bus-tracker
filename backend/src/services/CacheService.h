#pragma once

#include <functional>
#include <optional>
#include <string>
#include <json/json.h>

// Thin wrapper around Drogon's built-in Redis client (drogon::nosql),
// used as a cache-aside layer in front of Postgres for hot, read-heavy
// paths — currently just "the latest known location for a bus", which
// is polled by the frontend map and read on every WebSocket subscribe.
//
// Design notes:
//  - Redis is a cache, not a source of truth. Every write here happens
//    alongside (never instead of) the corresponding Postgres write in
//    TrackingRepo, and every read here has a Postgres fallback.
//  - If the "default" redis client isn't configured, or Redis is briefly
//    unreachable, calls degrade to a cache miss rather than an error —
//    callers should always be written to treat "no cache client" and
//    "cache miss" identically, so the app keeps working (just slower,
//    hitting Postgres) if Redis is down or not configured.
class CacheService
{
public:
    // Caches the latest location payload for a bus (same JSON shape
    // returned by GET /api/tracking/:busId/latest). Fire-and-forget:
    // failures are logged, never surfaced to the caller.
    static void cacheLatestLocation(const std::string& busId, const Json::Value& payload);

    // Looks up the cached latest location for a bus. Calls back with
    // std::nullopt on a cache miss, a disabled/unreachable cache, or a
    // malformed cached value — callers should fall back to Postgres.
    static void getCachedLatestLocation(const std::string& busId,
                                         std::function<void(std::optional<Json::Value>)> callback);

private:
    static std::string keyFor(const std::string& busId);
};
