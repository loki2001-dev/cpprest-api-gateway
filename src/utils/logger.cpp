#include "logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace utils {
    void init_logger() {
        try {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("api_gateway.log", true);

            spdlog::logger logger("multi_sink", {console_sink, file_sink});
            logger.set_level(spdlog::level::debug);
            spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));
            spdlog::flush_on(spdlog::level::info);

            spdlog::info("Logger initialized.");
        } catch (const spdlog::spdlog_ex &ex) {
            fprintf(stderr, "Log initialization failed: %s\n", ex.what());
        }
    }
}