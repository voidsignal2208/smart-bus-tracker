#include "JwtAuthFilter.h"
#include "../utils/JwtUtils.h"

using namespace drogon;

namespace
{
HttpResponsePtr unauthorized(const std::string& message)
{
    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(k401Unauthorized);
    resp->setContentTypeCode(CT_APPLICATION_JSON);
    resp->setBody("{\"error\": \"" + message + "\"}");
    return resp;
}
}  

void JwtAuthFilter::doFilter(const HttpRequestPtr& req,
                             FilterCallback&& fcb,
                             FilterChainCallback&& fcc) {

    
    std::string authHeader = req->getHeader("Authorization");

    
    if (authHeader.empty() || authHeader.rfind("Bearer ", 0) != 0) {
        fcb(unauthorized("Missing or invalid Authorization header"));
        return;
    }

    
    std::string token = authHeader.substr(7);

    
    auto claims = JwtUtils::verifyToken(token);

    if (!claims.has_value()) {
        fcb(unauthorized("Token is expired or invalid"));
        return;
    }

    
    
    req->getAttributes()->insert("user_id", claims->userId);
    req->getAttributes()->insert("role_id", claims->roleId);

    
    fcc();
}
