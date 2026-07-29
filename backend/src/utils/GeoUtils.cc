#include "GeoUtils.h"
#include <cmath>

namespace
{
// M_PI is a POSIX/GNU extension, not standard C++ — MSVC only defines it
// if _USE_MATH_DEFINES is set before every single prior inclusion of
// <cmath> in the translation unit (including via precompiled headers),
// which is fragile. Defining our own constant sidesteps that entirely.
constexpr double kPi = 3.14159265358979323846;
}  // namespace

double GeoUtils::toRadians(double degrees)
{
    return degrees * kPi / 180.0;
}

double GeoUtils::distanceMeters(double lat1, double lon1, double lat2, double lon2)
{
    double dLat = toRadians(lat2 - lat1);
    double dLon = toRadians(lon2 - lon1);

    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(toRadians(lat1)) * std::cos(toRadians(lat2)) *
               std::sin(dLon / 2) * std::sin(dLon / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));

    return kEarthRadiusMeters * c;
}

double GeoUtils::speedKmh(double lat1, double lon1, double lat2, double lon2, double seconds)
{
    if (seconds <= 0) return 0.0;
    double meters = distanceMeters(lat1, lon1, lat2, lon2);
    double metersPerSecond = meters / seconds;
    return metersPerSecond * 3.6;
}

double GeoUtils::etaSeconds(double currentLat, double currentLon,
                             double destLat, double destLon,
                             double speedKmh)
{
    if (speedKmh <= 0) return -1.0;
    double meters = distanceMeters(currentLat, currentLon, destLat, destLon);
    double metersPerSecond = speedKmh / 3.6;
    return meters / metersPerSecond;
}
