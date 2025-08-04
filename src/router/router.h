#ifndef CPPREST_API_GATEWAY_ROUTER_H
#define CPPREST_API_GATEWAY_ROUTER_H

#pragma once

#include <cpprest/http_msg.h>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>

struct RouteEntry {
    std::string target;
    std::string description;
    web::json::value static_response;
};

using MethodMap = std::unordered_map<std::string, RouteEntry>;

class Router final {
public:
    explicit Router();
    virtual ~Router();

    bool load_routes(const std::string& filename);
    std::optional<RouteEntry> match(const std::string& path, const std::string& method, std::map<std::string, std::string>& params) const;

private:
    std::map<std::string, MethodMap> _routes;

    static bool match_pattern(const std::string& route_pattern, const std::string& path, std::map<std::string, std::string>& params) ;
};

#endif //CPPREST_API_GATEWAY_ROUTER_H