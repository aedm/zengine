cmake_minimum_required (VERSION 3.8)

message(STATUS "Host: ${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}")

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_library(zengine STATIC ${SOURCE_FILES})

include_directories(zengine ${CMAKE_CURRENT_SOURCE_DIR})

#set(VCPKG_LIBRARY_LINKAGE static)
find_package(glm CONFIG REQUIRED)
find_package(rapidjson REQUIRED)
find_package(GLEW REQUIRED)
target_link_libraries(zengine PRIVATE glm)

# specify the C++ standard
set_property(TARGET zengine PROPERTY CXX_STANDARD 17)

target_include_directories (zengine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
