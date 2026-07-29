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
}  // namespace

void JwtAuthFilter::doFilter(const HttpRequestPtr& req,
                             FilterCallback&& fcb,
                             FilterChainCallback&& fcc) {

    // 1. Get the Authorization Header
    std::string authHeader = req->getHeader("Authorization");

    // 2. Check if it exists and starts with "Bearer "
    if (authHeader.empty() || authHeader.rfind("Bearer ", 0) != 0) {
        fcb(unauthorized("Missing or invalid Authorization header"));
        return;
    }

    // 3. Extract the actual token string
    std::string token = authHeader.substr(7);

    // 4. Verify the token using our utility class
    auto claims = JwtUtils::verifyToken(token);

    if (!claims.has_value()) {
        fcb(unauthorized("Token is expired or invalid"));
        return;
    }

    // 5. Success! Inject user_id and role_id into request attributes so
    // controllers (and RoleAuthFilter) can use them without re-decoding.
    req->getAttributes()->insert("user_id", claims->userId);
    req->getAttributes()->insert("role_id", claims->roleId);

    // Continue to the Controller
    fcc();
}
