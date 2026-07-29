#include "TrackingRepo.h"

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon_model::postgres;

void TrackingRepo::insertLocation(const std::string& busId,
                                   double latitude,
                                   double longitude,
                                   std::optional<double> speedKmh,
                                   std::function<void(const LocationRow&)> onSuccess,
                                   std::function<void(const std::string&)> onError)
{
    BusLocations loc;
    loc.setBusId(busId);
    loc.setLatitude(std::to_string(latitude));
    loc.setLongitude(std::to_string(longitude));
    if (speedKmh.has_value())
    {
        loc.setSpeed(std::to_string(*speedKmh));
    }

    auto dbClient = app().getDbClient();
    orm::Mapper<BusLocations> mapper(dbClient);

    mapper.insert(loc,
        [onSuccess](const BusLocations& inserted) { onSuccess(inserted); },
        [onError](const orm::DrogonDbException& e) {
            LOG_ERROR << "insertLocation failed: " << e.base().what();
            onError("Failed to record location (bus_id may not exist, or the data was rejected by the database)");
        });
}

void TrackingRepo::getLatestLocation(const std::string& busId,
                                      std::function<void(std::optional<LocationRow>)> onResult,
                                      std::function<void(const std::string&)> onError)
{
    auto dbClient = app().getDbClient();
    orm::Mapper<BusLocations> mapper(dbClient);

    mapper.orderBy(BusLocations::Cols::_timestamp, orm::SortOrder::DESC)
        .limit(1)
        .findBy(orm::Criteria(BusLocations::Cols::_bus_id, orm::CompareOperator::EQ, busId),
            [onResult](const std::vector<BusLocations>& rows) {
                if (rows.empty())
                {
                    onResult(std::nullopt);
                }
                else
                {
                    onResult(rows.front());
                }
            },
            [onError](const orm::DrogonDbException& e) {
                onError("Failed to fetch latest location");
            });
}

void TrackingRepo::getLocationHistory(const std::string& busId,
                                       int limit,
                                       std::function<void(const std::vector<LocationRow>&)> onResult,
                                       std::function<void(const std::string&)> onError)
{
    auto dbClient = app().getDbClient();
    orm::Mapper<BusLocations> mapper(dbClient);

    mapper.orderBy(BusLocations::Cols::_timestamp, orm::SortOrder::DESC)
        .limit(limit)
        .findBy(orm::Criteria(BusLocations::Cols::_bus_id, orm::CompareOperator::EQ, busId),
            [onResult](const std::vector<BusLocations>& rows) { onResult(rows); },
            [onError](const orm::DrogonDbException& e) {
                onError("Failed to fetch location history");
            });
}
