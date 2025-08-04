#ifndef CPPREST_API_GATEWAY_CACHE_H
#define CPPREST_API_GATEWAY_CACHE_H

#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <list>
#include <optional>
#include <mutex>
#include <cpprest/http_msg.h>

class Cache {
public:
    explicit Cache(size_t max_size = 10, std::chrono::seconds ttl = std::chrono::seconds(60));
    virtual ~Cache();

    std::optional<web::http::http_response> get(const std::string& key);
    void put(const std::string& key, const web::http::http_response& response);

    void cleanup_expired();

private:
    struct CacheEntry {
        web::http::http_response response;
        std::chrono::steady_clock::time_point expire_time;
    };

    void cleaner_thread();

    bool _running;
    std::thread _cleaner_thread;

    size_t _max_size;
    std::chrono::seconds _ttl;

    std::unordered_map<std::string, std::pair<CacheEntry, std::list<std::string>::iterator>> _cache_map;
    std::list<std::string> _lru_list;

    std::mutex _mutex;
};

#endif //CPPREST_API_GATEWAY_CACHE_H