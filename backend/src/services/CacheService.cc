#include "CacheService.h"

#include <drogon/drogon.h>
#include <drogon/nosql/RedisClient.h>
#include <sstream>

using namespace drogon;
using namespace drogon::nosql;

namespace
{
// Short TTL: this cache exists to absorb bursts of reads (map polling,
// many WebSocket clients subscribing at once) between location updates,
// not to serve stale positions. If a bus stops sending updates entirely,
// callers should fall through to Postgres (still has the last known row)
// rather than keep serving an ever-staler cached point forever.
constexpr int kLatestLocationTtlSeconds = 30;

std::string toCompactJson(const Json::Value& value)
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, value);
}

// getRedisClient() throws if the named client isn't configured (e.g. this
// deployment doesn't set REDIS_HOST). Treat that identically to "Redis is
// currently unreachable": callers fall back to Postgres either way.
RedisClientPtr tryGetRedisClient()
{
    try
    {
        return drogon::app().getRedisClient("default");
    }
    catch (const std::exception&)
    {
        return nullptr;
    }
}
}  // namespace

std::string CacheService::keyFor(const std::string& busId)
{
    return "bus:latest_location:" + busId;
}

void CacheService::cacheLatestLocation(const std::string& busId, const Json::Value& payload)
{
    auto client = tryGetRedisClient();
    if (!client)
    {
        return;
    }

    const std::string key = keyFor(busId);
    const std::string value = toCompactJson(payload);

    client->execCommandAsync(
        [](const RedisResult&) {
            // Fire-and-forget: nothing to do on success.
        },
        [key](const RedisException& err) {
            LOG_WARN << "Redis SET failed for " << key << ": " << err.what();
        },
        "SET %s %s EX %d", key.c_str(), value.c_str(), kLatestLocationTtlSeconds);
}

void CacheService::getCachedLatestLocation(const std::string& busId,
                                            std::function<void(std::optional<Json::Value>)> callback)
{
    auto client = tryGetRedisClient();
    if (!client)
    {
        callback(std::nullopt);
        return;
    }

    const std::string key = keyFor(busId);

    client->execCommandAsync(
        [key, callback](const RedisResult& result) {
            if (result.type() != RedisResultType::kString)
            {
                // kNil (cache miss) or an unexpected type — either way,
                // the caller should fall back to Postgres.
                callback(std::nullopt);
                return;
            }

            Json::Value parsed;
            Json::CharReaderBuilder builder;
            std::string errs;
            std::istringstream iss(result.asString());
            if (!Json::parseFromStream(builder, iss, &parsed, &errs))
            {
                LOG_WARN << "Discarding malformed cached JSON for " << key << ": " << errs;
                callback(std::nullopt);
                return;
            }

            callback(parsed);
        },
        [key, callback](const RedisException& err) {
            LOG_WARN << "Redis GET failed for " << key << ": " << err.what();
            callback(std::nullopt);
        },
        "GET %s", key.c_str());
}
