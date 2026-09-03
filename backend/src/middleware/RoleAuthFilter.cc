#include "RoleAuthFilter.h"
#include "../utils/Roles.h"

using namespace drogon;

namespace
{
HttpResponsePtr forbidden(const std::string& message)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k403Forbidden);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    resp->setBody("{\"error\": \"" + message + "\"}");
    return resp;
}

HttpResponsePtr serverMisconfigured()
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k500InternalServerError);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    resp->setBody("{\"error\": \"Role check ran before authentication\"}");
    return resp;
}
}  

void AdminOnlyFilter::doFilter(const HttpRequestPtr& req,
                                FilterCallback&& fcb,
                                FilterChainCallback&& fcc)
{
    if (!req->getAttributes()->find("role_id"))
    {
        fcb(serverMisconfigured());
        return;
    }

    int roleId = req->getAttributes()->get<int>("role_id");
    if (roleId != Roles::ADMIN)
    {
        fcb(forbidden("This action requires an administrator account"));
        return;
    }

    fcc();
}

void StaffOnlyFilter::doFilter(const HttpRequestPtr& req,
                                FilterCallback&& fcb,
                                FilterChainCallback&& fcc)
{
    if (!req->getAttributes()->find("role_id"))
    {
        fcb(serverMisconfigured());
        return;
    }

    int roleId = req->getAttributes()->get<int>("role_id");
    if (roleId != Roles::DRIVER && roleId != Roles::CONDUCTOR && roleId != Roles::ADMIN)
    {
        fcb(forbidden("This action requires a driver, conductor, or administrator account"));
        return;
    }

    fcc();
}
