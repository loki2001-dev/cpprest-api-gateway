#include "gateway_server.h"
#include <cpprest/http_client.h>
#include <spdlog/spdlog.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

GatewayServer::GatewayServer(const std::string& address, const std::string& route_config_file)
    : listener_(uri(address)),
    _cache(10, std::chrono::seconds(60))
{
    if (!_router.load_routes(route_config_file)) {
        spdlog::critical("Failed to load routes from file: {}", route_config_file);
    }

    spdlog::info("Route configuration loaded successfully from {}", route_config_file);

    listener_.support(methods::GET, [this](auto && PH1) { handle_request(std::forward<decltype(PH1)>(PH1)); });
    listener_.support(methods::POST, [this](auto && PH1) { handle_request(std::forward<decltype(PH1)>(PH1)); });
    listener_.support(methods::PUT, [this](auto && PH1) { handle_request(std::forward<decltype(PH1)>(PH1)); });
    listener_.support(methods::DEL, [this](auto && PH1) { handle_request(std::forward<decltype(PH1)>(PH1)); });
    listener_.support(methods::OPTIONS, [](http_request req) {
        // CORS
        req.reply(status_codes::OK)
            .then([&req](const pplx::task<void>&) {
                req.headers().add(U("Access-Control-Allow-Origin"), U("*"));
                req.headers().add(U("Access-Control-Allow-Methods"), U("GET, POST, PUT, DELETE, OPTIONS"));
                req.headers().add(U("Access-Control-Allow-Headers"), U("Content-Type, Accept"));
            });
        spdlog::debug("Handled CORS preflight for {}", req.relative_uri().to_string());
    });

    spdlog::info("GatewayServer initialized and ready to accept requests at {}", address);
}

GatewayServer::~GatewayServer(){
    spdlog::info("GatewayServer shutting down.");
}

pplx::task<void> GatewayServer::open() {
    spdlog::info("Starting API Gateway on {}", listener_.uri().to_string());
    return listener_.open();
}

pplx::task<void> GatewayServer::close() {
    spdlog::info("Stopping API Gateway on {}", listener_.uri().to_string());
    return listener_.close();
}

void GatewayServer::send_error(const http_request& request, const std::string& message) {
    json::value error_json;
    error_json[U("error")] = json::value::string(U(message));
    request.reply(500, error_json);
    spdlog::warn("Sent error response: {} - {}", 500, message);
}

void GatewayServer::setup_cors(http_request& request) {
    request.headers().add(U("Access-Control-Allow-Origin"), U("*"));
}

void GatewayServer::handle_request(http_request request) {
    setup_cors(request);
    auto path = request.relative_uri().path();
    auto method = request.method();

    spdlog::info("Incoming request: {} {}", method, path);

    // Router matching
    std::map<std::string, std::string> params;
    const auto route_opt = _router.match(path, method, params);
    if (!route_opt) {
        spdlog::warn("No route matched for {} {}", method, path);
        send_error(request, "Route not found");
        return;
    }

    // HEALTH CHECK
    const auto& route = *route_opt;
    if (route.target == "static_response") {
        request.reply(status_codes::OK, route.static_response);
        spdlog::info("Served static response for {} {}", method, path);
        return;
    }

    // Cache key = METHOD: FULL_PATH (after path parameter substitution)
    std::string final_target = route.target;
    // Replace path parameters in target
    for (auto& [k, v] : params) {
        std::string placeholder = "{" + k + "}";
        size_t pos = 0;
        while ((pos = final_target.find(placeholder, pos)) != std::string::npos) {
            final_target.replace(pos, placeholder.size(), v);
            pos += v.size();
        }
    }

    // CACHING
    std::string cache_key = method + ":" + path;
    if (method == methods::GET || method == methods::POST || method == methods::PUT || method == methods::DEL || method == methods::OPTIONS) {
        auto cached_response_opt = _cache.get(cache_key);
        if (cached_response_opt) {
            spdlog::debug("Cache HIT: {}", cache_key);
            auto status = cached_response_opt->status_code();
            auto body = cached_response_opt->extract_string().get();
            request.reply(status, body);
            return;
        }
        spdlog::debug("Cache MISS: {}", cache_key);
    }

    // REQUEST (ASYNC)
    web::http::client::http_client client(U(final_target.substr(0, final_target.find('/', 7))));
    auto relative_path = final_target.substr(final_target.find('/', 7));

    http_request backend_req(method);
    backend_req.set_request_uri(relative_path);

    // HEADER
    for (auto& h : request.headers()) {
        backend_req.headers().add(h.first, h.second);
    }

    // BODY
    if (method == methods::POST || method == methods::PUT) {
        try {
            auto body = request.extract_string().get();
            backend_req.set_body(body);
            spdlog::debug("Copied request body for {} {}", method, path);
        } catch (...) {
            spdlog::warn("Failed to extract request body for {} {}", method, path);
        }
    }

    client.request(backend_req).then([this, request](const pplx::task<http_response>& task) mutable {
        try {
            auto resp = task.get();
            auto status = resp.status_code();
            auto resp_body = resp.extract_string().get();

            spdlog::info("Forwarded request to backend: {} (Status: {})", request.relative_uri().to_string(), status);

            if (request.method() == methods::GET && status == status_codes::OK) {
                // Store in cache
                http_response cache_resp(status);
                cache_resp.set_body(resp_body);
                _cache.put(request.method() + ":" + request.relative_uri().to_string(), cache_resp);
                spdlog::debug("Stored response in cache for key: {}", request.method() + ":" + request.relative_uri().to_string());
            }

            request.reply(status, resp_body);
        } catch (const std::exception& e) {
            spdlog::error("Backend request failed: {}", e.what());
            send_error(request, "Backend request failed");
        }
    });
}