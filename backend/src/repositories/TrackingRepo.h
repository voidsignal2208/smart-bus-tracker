#pragma once

#include "../models/BusLocations.h"
#include <functional>
#include <optional>
#include <vector>





class TrackingRepo
{
public:
    using LocationRow = drogon_model::postgres::BusLocations;

    
    static void insertLocation(const std::string& busId,
                                double latitude,
                                double longitude,
                                std::optional<double> speedKmh,
                                std::function<void(const LocationRow&)> onSuccess,
                                std::function<void(const std::string&)> onError);

    
    static void getLatestLocation(const std::string& busId,
                                   std::function<void(std::optional<LocationRow>)> onResult,
                                   std::function<void(const std::string&)> onError);

    
    static void getLocationHistory(const std::string& busId,
                                    int limit,
                                    std::function<void(const std::vector<LocationRow>&)> onResult,
                                    std::function<void(const std::string&)> onError);
};
