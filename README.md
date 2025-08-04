# cpprest-api-gateway (C++20 & cpprestsdk)
- A lightweight and efficient API Gateway server built with modern C++20.
- Handles client requests by routing them to appropriate backend services and caching responses to improve performance.
- Designed for MSA(microservices architectures) to facilitate request routing, authentication, logging, and caching.

---

## Features
- Supports HTTP methods: GET, POST, PUT, DELETE, OPTIONS
- JSON-based routing configuration
- Path parameter substitution for flexible route matching
- Static response support for health checks and simple endpoints
- CORS request handling
- Asynchronous RESTful API Server request forwarding
- Response caching with configurable TTL to reduce backend load
- Detailed logging with spdlog for monitoring and debugging

---

## API Routing

| API Endpoint      | Method | Target URL                              | Description                        |
|-------------------|--------|-----------------------------------------|------------------------------------|
| `/api/users`      | GET    | http://localhost:9001/users             | Retrieve list of all users         |
| `/api/users`      | POST   | http://localhost:9001/users/create      | Create a new user                  |
| `/api/users/{id}` | GET    | http://localhost:9001/users/{id}        | Retrieve specific user             |
| `/api/users/{id}` | PUT    | http://localhost:9001/users/{id}/update | Update user information            |
| `/api/users/{id}` | DELETE | http://localhost:9001/users/{id}/delete | Delete a user                      |
| `/api/products`   | GET    | http://localhost:9002/products          | Retrieve list of all products      |
| `/api/orders`     | POST   | http://localhost:9003/orders/create     | Create an order                    |
| `/healthz`        | GET    | static response                         | Health check response (status: ok) |

---

## Getting Started
### Prerequisites
- Linux (Ubuntu 20.04 or later recommended)
- Requires CMake 3.14 or later
- Requires C++20 or later compiler
- [cpprestsdk](https://github.com/microsoft/cpprestsdk) (included as a submodule)
- [spdlog](https://github.com/gabime/spdlog) (included as a submodule)
- [Catch2](https://github.com/catchorg/Catch2) (included as a submodule)

---

## Build Instructions
### Setup and Installation
```bash
# Update package lists
sudo apt update

# Install dependencies
sudo apt install -y libssl-dev
sudo apt install -y python3-flask

# Clone the repository
git clone https://github.com/loki2001-dev/cpprest-api-gateway.git
cd cpprest-api-gateway

# Initialize submodules (spdlog, Catch2)
git submodule update --init --recursive

# Install dependencies
. install_cpprest_sdk.sh

# Build the project
. build_project.sh

```

## Testing (TDD with Catch2)
- This project follows Test-Driven Development (TDD) practices using Catch2, a modern C++ unit testing framework.

### Test Structure
- All test sources are located under the tests/ directory.
- The test logic is completely separated from the main application build.

- Build outputs are organized as follows:
  - Main application: build/api_gateway
  - Test application: build/tests/api_gateway_run_tests