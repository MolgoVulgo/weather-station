# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)
FILE(GLOB_RECURSE weather_sources ${CMAKE_SOURCE_DIR}/libraries/weather/*.cpp)

idf_component_register(
    SRCS ${app_sources} ${weather_sources}
    INCLUDE_DIRS
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/ui
        ${CMAKE_SOURCE_DIR}/libraries/weather
    REQUIRES spiffs esp_http_client esp-tls mbedtls json
)
