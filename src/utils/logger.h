#ifndef CPPREST_API_GATEWAY_LOGGER_H
#define CPPREST_API_GATEWAY_LOGGER_H

#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace utils {
    void init_logger();
    inline std::shared_ptr<spdlog::logger> get_logger() {
        return spdlog::default_logger();
    }
}

#endif //CPPREST_API_GATEWAY_LOGGER_H