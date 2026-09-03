#pragma once








class GeoUtils
{
public:
    static constexpr double kEarthRadiusMeters = 6371000.0;

    
    static double distanceMeters(double lat1, double lon1, double lat2, double lon2);

    
    
    
    static double speedKmh(double lat1, double lon1, double lat2, double lon2, double seconds);

    
    
    
    static double etaSeconds(double currentLat, double currentLon,
                              double destLat, double destLon,
                              double speedKmh);

private:
    static double toRadians(double degrees);
};
