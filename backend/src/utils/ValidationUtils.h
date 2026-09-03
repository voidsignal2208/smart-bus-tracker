#pragma once

#include <string>
#include <regex>

class ValidationUtils
{
public:
    
    static bool isValidUuid(const std::string& value)
    {
        static const std::regex uuidPattern(
            "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-"
            "[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
        return std::regex_match(value, uuidPattern);
    }

    
    
    static bool isValidEmail(const std::string& value)
    {
        static const std::regex emailPattern(
            R"(^[^\s@]+@[^\s@]+\.[^\s@]+$)");
        return std::regex_match(value, emailPattern) && value.size() <= 150;
    }

    static bool isValidLatitude(double lat)
    {
        return lat >= -90.0 && lat <= 90.0;
    }

    static bool isValidLongitude(double lon)
    {
        return lon >= -180.0 && lon <= 180.0;
    }
};
