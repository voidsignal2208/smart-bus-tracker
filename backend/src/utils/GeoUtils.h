#pragma once

// GeoUtils provides the geometric calculations the tracking feature needs:
// distance between two lat/lon points, speed derived from two timestamped
// points, and a naive straight-line ETA estimate.
//
// These are intentionally simple (great-circle distance, not road-network
// routing) — good enough for "how far is the bus" and "roughly when will
// it arrive" without pulling in a routing engine dependency.
class GeoUtils
{
public:
    static constexpr double kEarthRadiusMeters = 6371000.0;

    // Great-circle (haversine) distance between two points, in meters.
    static double distanceMeters(double lat1, double lon1, double lat2, double lon2);

    // Speed in km/h implied by moving between two timestamped points.
    // `seconds` must be > 0; returns 0 if seconds <= 0 to avoid division by zero
    // or nonsensical negative-time speeds.
    static double speedKmh(double lat1, double lon1, double lat2, double lon2, double seconds);

    // Very rough ETA (in seconds) to reach (destLat, destLon) at the given
    // speed (km/h). Returns -1 if speedKmh <= 0 (can't estimate with no
    // motion / unknown speed).
    static double etaSeconds(double currentLat, double currentLon,
                              double destLat, double destLon,
                              double speedKmh);

private:
    static double toRadians(double degrees);
};
