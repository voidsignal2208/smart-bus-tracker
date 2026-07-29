#pragma once

#include <drogon/HttpFilter.h>

// Must be registered AFTER JwtAuthFilter in a controller's filter list,
// since it relies on "role_id" already being present in request attributes.
//
// Only ADMIN accounts may pass.
class AdminOnlyFilter : public drogon::HttpFilter<AdminOnlyFilter>
{
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fcc) override;
};

// Only DRIVER, CONDUCTOR, or ADMIN accounts may pass (i.e. any staff role,
// excluding plain PASSENGER accounts). Used for endpoints that push bus
// location updates.
class StaffOnlyFilter : public drogon::HttpFilter<StaffOnlyFilter>
{
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fcc) override;
};
