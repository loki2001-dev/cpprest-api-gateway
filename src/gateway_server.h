#ifndef CPPREST_API_GATEWAY_GATEWAY_SERVER_H
#define CPPREST_API_GATEWAY_GATEWAY_SERVER_H

#pragma once

#include <cpprest/http_listener.h>
#include "router/router.h"
#include "cache/cache.h"

class GatewayServer final {
public:
    explicit GatewayServer(const std::string& address, const std::string& route_config_file);
    virtual ~GatewayServer();

    pplx::task<void> open();
    pplx::task<void> close();

private:
    web::http::experimental::listener::http_listener listener_;
    Router _router;
    Cache _cache;

    void handle_request(web::http::http_request request);
    static void send_error(const web::http::http_request & request, const std::string& message);
    static void setup_cors(web::http::http_request& request);
};

#endif //CPPREST_API_GATEWAY_GATEWAY_SERVER_H