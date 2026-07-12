#pragma once

#include <string>
#include <regex>

class ValidationUtils
{
public:
    // Accepts standard UUID v1-v5 textual form: 8-4-4-4-12 hex digits.
    static bool isValidUuid(const std::string& value)
    {
        static const std::regex uuidPattern(
            "^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-"
            "[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
        return std::regex_match(value, uuidPattern);
    }

    // Deliberately simple RFC-5322-ish check: good enough to reject
    // obviously malformed input without rejecting valid edge-case emails.
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
