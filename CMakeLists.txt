cmake_minimum_required(VERSION 3.12)
set(EXTENSION kxnrl.message)
project(${EXTENSION})

set(CMAKE_CXX_STANDARD 17)

set(HL2SDK_PATH CACHE PATH "${CMAKE_SOURCE_DIR}/hl2sdk-csgo")
set(SOURCEMOD_PATH CACHE PATH "${CMAKE_SOURCE_DIR}/sourcemod")
set(METAMOD_SOURCE_PATH CACHE PATH "${CMAKE_SOURCE_DIR}/metamod-source")
include(cmake/smsdk_ext.cmake)

find_package(Boost REQUIRED coroutine thread)
find_package(OpenSSL REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)

add_library(${EXTENSION} SHARED
        extension.cpp
        natives.cpp
        websocket.cpp
        )

target_include_directories(${EXTENSION} PUBLIC
        ./
        )

target_link_libraries(${EXTENSION} PUBLIC smsdk_ext Boost::headers Boost::coroutine Boost::thread OpenSSL::SSL jsoncpp_lib)
set_target_properties(${EXTENSION} PROPERTIES PREFIX "" SUFFIX ".ext${HL2SDK_LIB_SHARED_EXT}")