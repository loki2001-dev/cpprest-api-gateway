#include "cache.h"
#include <spdlog/spdlog.h>

Cache::Cache(size_t max_size, std::chrono::seconds ttl)
    : _running(true),
      _max_size(max_size),
      _ttl(ttl) {
    _cleaner_thread = std::thread(&Cache::cleaner_thread, this);
    spdlog::info("Cache initialized with max_size={} and ttl={}s", _max_size, _ttl.count());
}

Cache::~Cache() {
    _running = false;
    if (_cleaner_thread.joinable()) {
        _cleaner_thread.join();
    }
    spdlog::info("Cache cleaner thread stopped.");
}

std::optional<web::http::http_response> Cache::get(const std::string& key) {
    std::lock_guard lock(_mutex);
    auto it = _cache_map.find(key);
    if (it == _cache_map.end()) {
        spdlog::debug("Cache MISS: {}", key);
        return std::nullopt;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now > it->second.first.expire_time) {
        spdlog::debug("Cache EXPIRED: {}", key);
        _lru_list.erase(it->second.second);
        _cache_map.erase(it);
        return std::nullopt;
    }

    // LRU
    _lru_list.splice(_lru_list.begin(), _lru_list, it->second.second);
    spdlog::debug("Cache HIT: {}", key);
    return it->second.first.response;
}

void Cache::put(const std::string& key, const web::http::http_response& response) {
    std::lock_guard lock(_mutex);
    auto now = std::chrono::steady_clock::now();
    auto expire = now + _ttl;

    auto it = _cache_map.find(key);
    if (it != _cache_map.end()) {
        // UPDATE
        it->second.first.response = response;
        it->second.first.expire_time = expire;
        _lru_list.splice(_lru_list.begin(), _lru_list, it->second.second);
        spdlog::debug("Cache UPDATED: {}", key);
        return;
    }

    // INSERT
    _lru_list.push_front(key);
    _cache_map[key] = { {response, expire}, _lru_list.begin() };
    spdlog::debug("Cache INSERTED: {}", key);

    if (_cache_map.size() > _max_size) {
        // DELETE Last LRU
        std::string lru_key = _lru_list.back();
        _lru_list.pop_back();
        _cache_map.erase(lru_key);
        spdlog::debug("Cache EVICTED (LRU): {}", lru_key);
    }
}

void Cache::cleanup_expired() {
    std::lock_guard lock(_mutex);
    const auto now = std::chrono::steady_clock::now();
    size_t removed = 0;

    for (auto it = _cache_map.begin(); it != _cache_map.end();) {
        if (now > it->second.first.expire_time) {
            spdlog::debug("Cache CLEANUP removed expired key: {}", *it->second.second);
            _lru_list.erase(it->second.second);
            it = _cache_map.erase(it);
            removed++;
        } else {
            ++it;
        }
    }

    if (removed > 0) {
        spdlog::info("Cache cleanup: {} expired entries removed", removed);
    }
}

void Cache::cleaner_thread() {
    spdlog::info("Cache cleaner thread started.");
    while (_running) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        cleanup_expired();
    }
}