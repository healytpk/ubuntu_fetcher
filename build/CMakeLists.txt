cmake_minimum_required(VERSION 3.10)

project(ubuntu_fetcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}" ${CMAKE_MODULE_PATH})

find_package(CURL   REQUIRED)
find_package(JSON-C REQUIRED)

add_executable(ubuntu_fetcher
    ../main.cpp
    ../distro_fetcher.cpp
)

target_include_directories(ubuntu_fetcher
    PRIVATE
    ../
    ${CURL_INCLUDE_DIRS}
    ${JSON-C_INCLUDE_DIRS}
)

target_link_libraries(ubuntu_fetcher
    PRIVATE
    ${CURL_LIBRARIES}
    ${JSON-C_LIBRARIES}
)

target_compile_definitions(ubuntu_fetcher PRIVATE JSON_STATIC)
