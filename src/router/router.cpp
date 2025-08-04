#include "router.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <cpprest/json.h>
#include <spdlog/spdlog.h>

Router::Router()
    : _routes{} {
}

Router::~Router() {
}

bool Router::load_routes(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
        spdlog::info("Failed to open routes file: {}", filename);
        return false;
    }
    try {
        auto json = web::json::value::parse(ifs);
        for (auto& route_pair : json.as_object()) {
            const std::string& route_path = route_pair.first;
            auto& methods_obj = route_pair.second.as_object();

            MethodMap method_map;
            for (auto& method_pair : methods_obj) {
                std::string method = method_pair.first;
                auto& route_info = method_pair.second;

                RouteEntry entry;
                entry.target = route_info.at(U("target")).as_string();
                entry.description = route_info.at(U("description")).as_string();

                // has_field 대신 아래와 같이 수정
                if (entry.target == "static_response" &&
                    route_info.is_object() &&
                    route_info.as_object().find(U("response")) != route_info.as_object().end()) {
                    entry.static_response = route_info.at(U("response"));
                }
                method_map.emplace(std::move(method), std::move(entry));
            }
            _routes.emplace(route_path, std::move(method_map));
        }
        spdlog::info("Loaded {} routes from {}", _routes.size(), filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error parsing routes.json: {}", e.what());
        return false;
    }
}

bool Router::match_pattern(const std::string& route_pattern, const std::string& path, std::map<std::string, std::string>& params) {
    std::string regex_str;
    std::vector<std::string> keys;
    size_t pos = 0;
    while (pos < route_pattern.size()) {
        if (route_pattern[pos] == '{') {
            size_t end = route_pattern.find('}', pos);
            if (end == std::string::npos) return false;
            std::string key = route_pattern.substr(pos+1, end - pos - 1);
            regex_str += "([^/]+)";
            keys.push_back(key);
            pos = end + 1;
        } else {
            if (std::string(".*+?^${}()|[]\\").find(route_pattern[pos]) != std::string::npos)
                regex_str += "\\";
            regex_str += route_pattern[pos];
            pos++;
        }
    }
    regex_str = "^" + regex_str + "$";

    std::regex re(regex_str);
    std::smatch match;
    if (!std::regex_match(path, match, re)) {
        return false;
    }
    if (match.size() - 1 != keys.size()) {
        return false;
    }

    for (size_t i = 0; i < keys.size(); ++i) {
        params[keys[i]] = match[i+1].str();
    }
    return true;
}

std::optional<RouteEntry> Router::match(const std::string& path, const std::string& method, std::map<std::string, std::string>& params) const {
    for (const auto& [route_pattern, methods] : _routes) {
        std::map<std::string, std::string> temp_params;
        if (match_pattern(route_pattern, path, temp_params)) {
            auto it = methods.find(method);
            if (it != methods.end()) {
                params = std::move(temp_params);
                return it->second;
            }
        }
    }
    return std::nullopt;
}