FILE(GLOB TEST_FILES *.cpp)

ADD_EXECUTABLE(api_gateway_run_tests
        ${TEST_FILES}
)

TARGET_INCLUDE_DIRECTORIES(api_gateway_run_tests PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/3rdparty/Catch2/single_include
)

TARGET_LINK_LIBRARIES(api_gateway_run_tests
        Catch2::Catch2WithMain
        spdlog::spdlog
)

ADD_TEST(NAME AllTests COMMAND api_gateway_run_tests)