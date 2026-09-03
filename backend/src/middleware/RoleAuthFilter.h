#pragma once

#include <drogon/HttpFilter.h>





class AdminOnlyFilter : public drogon::HttpFilter<AdminOnlyFilter>
{
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fcc) override;
};




class StaffOnlyFilter : public drogon::HttpFilter<StaffOnlyFilter>
{
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fcc) override;
};
