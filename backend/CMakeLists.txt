cmake_minimum_required(VERSION 3.16)
project(SmartBusTracker CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Drogon CONFIG REQUIRED)

if(APPLE)
    if(EXISTS "/opt/homebrew/opt/openssl@3")
        set(OPENSSL_ROOT_DIR /opt/homebrew/opt/openssl@3)
    elseif(EXISTS "/usr/local/opt/openssl@3")
        set(OPENSSL_ROOT_DIR /usr/local/opt/openssl@3)
    endif()
endif()

find_package(OpenSSL REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

include(FetchContent)

find_package(nlohmann_json CONFIG QUIET)
if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json not found via find_package(); fetching a modern release instead")
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

find_package(jwt-cpp CONFIG QUIET)
if(NOT jwt-cpp_FOUND)
    message(STATUS "jwt-cpp not found via find_package(); fetching jwt-cpp from source instead")

    set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        jwt-cpp
        GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
        GIT_TAG v0.7.0
    )

    FetchContent_MakeAvailable(jwt-cpp)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${OPENSSL_INCLUDE_DIR})

file(GLOB_RECURSE SRC_FILES
    "src/*.cc"
    "src/*.cpp"
    "main.cc"
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_precompile_headers(${PROJECT_NAME} PRIVATE
    <codecvt>
    <locale>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Drogon::Drogon
        OpenSSL::SSL
        OpenSSL::Crypto
        jwt-cpp::jwt-cpp
        nlohmann_json::nlohmann_json
        jsoncpp_lib
)