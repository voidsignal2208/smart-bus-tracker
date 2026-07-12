#pragma once

#include "../models/BusLocations.h"
#include <functional>
#include <optional>
#include <vector>

// Thin repository wrapping the Drogon ORM mapper for BusLocations.
// Kept separate from TrackingService so the service layer's business
// logic (speed computation, broadcasting) doesn't need to know about
// ORM/SQL details.
class TrackingRepo
{
public:
    using LocationRow = drogon_model::postgres::BusLocations;

    // Inserts a new location reading for a bus.
    static void insertLocation(const std::string& busId,
                                double latitude,
                                double longitude,
                                std::optional<double> speedKmh,
                                std::function<void(const LocationRow&)> onSuccess,
                                std::function<void(const std::string&)> onError);

    // Fetches the single most recent reading for a bus, if any.
    static void getLatestLocation(const std::string& busId,
                                   std::function<void(std::optional<LocationRow>)> onResult,
                                   std::function<void(const std::string&)> onError);

    // Fetches up to `limit` most recent readings for a bus, newest first.
    static void getLocationHistory(const std::string& busId,
                                    int limit,
                                    std::function<void(const std::vector<LocationRow>&)> onResult,
                                    std::function<void(const std::string&)> onError);
};
