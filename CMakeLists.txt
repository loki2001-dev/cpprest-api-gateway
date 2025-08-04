CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(cpprest-api-gateway)

# OPTION (Standard)
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
ADD_COMPILE_OPTIONS(-Wall -Wextra -Wpedantic)

IF(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    MESSAGE(STATUS "Detected compiler: GCC")
    ADD_COMPILE_OPTIONS(-std=c++20)
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    MESSAGE(STATUS "Detected compiler: Clang")
    ADD_COMPILE_OPTIONS(-std=c++20)
ENDIF()

# OPTION (3rdparty)
SET(BUILD_SHARED_LIBS ON)
SET(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/3rdparty)

# ADD 3rdparty
ADD_SUBDIRECTORY(${THIRDPARTY_DIR}/spdlog)
ADD_SUBDIRECTORY(${THIRDPARTY_DIR}/Catch2)

# FIND (cpprestsdk, OpenSSL)
FIND_PACKAGE(cpprestsdk REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)

# INCLUDE
SET(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
SET(TEST_DIR ${CMAKE_SOURCE_DIR}/tests)

INCLUDE_DIRECTORIES(
        ${SRC_DIR}
)

# GLOB (Sources)
FILE(GLOB_RECURSE SRC_FILES
        ${SRC_DIR}/*.cpp
        ${SRC_DIR}/*.h
)

FILE(GLOB TEST_FILES
        ${TEST_DIR}/*.cpp
)

# DEFINE executable file
ADD_EXECUTABLE(api_gateway
        ${SRC_FILES}
)

# INCLUDE
TARGET_INCLUDE_DIRECTORIES(api_gateway PRIVATE
        ${SRC_DIR}
)

# LINK
TARGET_LINK_LIBRARIES(api_gateway
        cpprest
        spdlog::spdlog
        OpenSSL::Crypto
        OpenSSL::SSL
)

# ENABLE testing
ENABLE_TESTING()

# DEFINE executable file
ADD_EXECUTABLE(api_gateway_run_tests
        ${TEST_FILES}
)

# INCLUDE
TARGET_INCLUDE_DIRECTORIES(api_gateway_run_tests PRIVATE
        ${SRC_DIR}
        ${TEST_DIR}
        ${THIRDPARTY_DIR}/catch2
)

# LINK
TARGET_LINK_LIBRARIES(api_gateway_run_tests
        Catch2::Catch2WithMain
        spdlog::spdlog
)

# TEST entry
ADD_TEST(NAME AllTests COMMAND api_gateway_run_tests)