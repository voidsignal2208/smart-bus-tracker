#include <drogon/drogon.h>
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "src/utils/EnvConfig.h"

namespace
{
// Reads config.json, expands ${VAR} placeholders using the process
// environment, parses the result, and coerces the fields that Drogon
// expects as JSON integers (but which arrive as strings after our text
// substitution) back into integers.
Json::Value loadAppConfig(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open config file: " + path);
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string expanded = EnvConfig::expandEnvVars(buffer.str());

    Json::CharReaderBuilder builder;
    Json::Value root;
    std::string errs;
    std::istringstream iss(expanded);
    if (!Json::parseFromStream(builder, iss, &root, &errs))
    {
        throw std::runtime_error("Failed to parse config.json after env expansion: " + errs);
    }

    // Coerce fields that must be JSON integers but came through as strings.
    if (root["listeners"].isArray())
    {
        for (auto& listener : root["listeners"])
        {
            if (listener["port"].isString())
            {
                listener["port"] = std::stoi(listener["port"].asString());
            }
        }
    }

    if (root["db_clients"].isArray())
    {
        for (auto& client : root["db_clients"])
        {
            if (client["port"].isString())
            {
                client["port"] = std::stoi(client["port"].asString());
            }
        }
    }

    return root;
}
}  // namespace

int main()
{
    std::cout << "Starting Smart Bus Tracker C++ Backend..." << std::endl;

    // Load local dev secrets from backend/.env if present. In real
    // deployments, environment variables are set by the platform instead
    // and this is a no-op (existing env vars are never overwritten).
    EnvConfig::loadDotEnv(".env");
    EnvConfig::loadDotEnv("../.env");
    EnvConfig::loadDotEnv("../../.env");
    EnvConfig::loadDotEnv("../../../.env");
    EnvConfig::loadDotEnv("../../../../.env");
    try
    {
        Json::Value config = loadAppConfig("config.json");
        drogon::app().loadConfigJson(config);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to load configuration: " << e.what() << std::endl;
        return 1;
    }

    // Public health-check route (no auth required) — useful for load
    // balancers / container orchestrators / uptime checks.
    drogon::app().registerHandler(
        "/api/health",
        [](const drogon::HttpRequestPtr& req,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value ret;
            ret["status"] = "ok";
            callback(drogon::HttpResponse::newHttpJsonResponse(ret));
        },
        {drogon::Get});

    drogon::app().run();

    return 0;
}