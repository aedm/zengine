cmake_minimum_required (VERSION 3.15)

message(STATUS "Host: ${CMAKE_HOST_SYSTEM_NAME}/${CMAKE_HOST_SYSTEM_PROCESSOR}")

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets REQUIRED)
find_package(assimp REQUIRED)
find_package(GLEW REQUIRED)

file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/forms/*.qrc)
file(GLOB_RECURSE UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/forms/*.ui)
add_executable(zengarden ${UI_FILES} ${RESOURCE_FILES} ${SOURCE_FILES})

set_property(TARGET zengarden PROPERTY CXX_STANDARD 17)

target_link_libraries(zengarden zengine Qt5::Widgets GLEW::GLEW assimp::assimp ${CMAKE_CURRENT_SOURCE_DIR}/../components/bass/bass.lib)

include_directories(zengarden ${CMAKE_CURRENT_SOURCE_DIR}/../components/bass)

# if (WIN32)
# set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
# set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
# endif ()