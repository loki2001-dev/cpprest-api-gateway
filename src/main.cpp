#include "gateway_server.h"
#include "utils/logger.h"
#include <iostream>
#include <atomic>
#include <array>
#include <chrono>
#include <csignal>
#include <memory>
#include <thread>
#include <string>

std::atomic<bool> running{true};

void signal_handler(int) {
    running = false;
}

const std::array<int, 6> handled_signals = {
    SIGINT, SIGTERM, SIGTSTP, SIGHUP, SIGQUIT, SIGUSR1
};

void initialize() {
    // signal
    for (int sig : handled_signals) {
        std::signal(sig, signal_handler);
    }

    // logger
    utils::init_logger();
}

int main() {
    try {
        GatewayServer server("http://0.0.0.0:8081", "../config/routes.json");
        server.open().wait();

        while (running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        server.close().wait();
    } catch (const std::exception& e) {
        spdlog::critical("Fatal error: {}", e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}