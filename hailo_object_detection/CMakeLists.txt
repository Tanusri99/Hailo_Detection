## Copyright 2018-present Network Optix, Inc. Licensed under MPL 2.0: www.mozilla.org/MPL/2.0/

cmake_minimum_required(VERSION 3.3.2)
project(hailo_object_detection_plugin)

set(metadataSdkDir "" CACHE PATH "Path to unpacked VMS Metadata SDK zip.")
if(metadataSdkDir STREQUAL "")
    set(metadataSdkDir ${CMAKE_CURRENT_LIST_DIR}/../metadata_sdk) #< Assume building samples inside the SDK.
    if(NOT EXISTS ${metadataSdkDir}/src/nx/sdk OR NOT EXISTS ${metadataSdkDir}/nx_kit/src/nx/kit)
        message(FATAL_ERROR "Define metadataSdkDir cache variable to point to the unzipped SDK.")
    endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX)
    # In Linux, for the plugin .so library, set `rpath` to "$ORIGIN" and do not set `runpath`, thus
    # enabling the lookup of the dependencies in the plugin dir first.
    string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--disable-new-dtags")
endif()
set(CMAKE_SKIP_BUILD_RPATH ON)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN")

if(WIN32)
    string(APPEND CMAKE_CXX_FLAGS " /MP") #< Use all CPU cores by MSVC.
    # Do not create separate .pdb files for object files (workaround for mspdbsrv.exe bug).
    add_compile_options("/Z7")
endif()

if(WIN32)
    set(API_EXPORT_MACRO "__declspec(dllexport)")
else()
    set(API_EXPORT_MACRO "__attribute__((visibility(\"default\")))")
endif()

#--------------------------------------------------------------------------------------------------
# Define nx_kit lib, static.

set(nxKitLibraryType "STATIC" CACHE STRING "" FORCE)
set(nxKitWithTests "NO" CACHE STRING "" FORCE)

add_subdirectory(${metadataSdkDir}/nx_kit ${CMAKE_CURRENT_BINARY_DIR}/nx_kit)

#--------------------------------------------------------------------------------------------------
# Define nx_sdk lib, static, depends on nx_kit.

set(SDK_SRC_DIR ${metadataSdkDir}/src)
file(GLOB_RECURSE SDK_SRC CONFIGURE_DEPENDS ${SDK_SRC_DIR}/*)

add_library(nx_sdk STATIC ${SDK_SRC} "src/nx/vms_server_plugins/analytics/sample/applogger.h" 
"src/nx/vms_server_plugins/analytics/sample/base64.h"
"src/nx/vms_server_plugins/analytics/sample/plugin_settings.h"
)
target_include_directories(nx_sdk PUBLIC ${SDK_SRC_DIR})
target_link_libraries(nx_sdk PRIVATE nx_kit)

target_compile_definitions(nx_sdk PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO}) #< for nxLibContext()

#--------------------------------------------------------------------------------------------------
# Define sample_analytics_plugin lib, dynamic, depends on nx_kit and nx_sdk.

set(hailo_object_detection_plugin_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
file(GLOB_RECURSE hailo_object_detection_plugin_SRC CONFIGURE_DEPENDS
    ${hailo_object_detection_plugin_SRC_DIR}/*)


add_library(hailo_object_detection_plugin SHARED ${hailo_object_detection_plugin_SRC}
"src/nx/vms_server_plugins/analytics/sample/applogger.h"
"src/nx/vms_server_plugins/analytics/sample/base64.h"
"src/nx/vms_server_plugins/analytics/sample/plugin_settings.h"
)

target_include_directories(hailo_object_detection_plugin PRIVATE ${hailo_object_detection_plugin_SRC_DIR})

find_package(CppKafka REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(CURL REQUIRED)
target_link_libraries(hailo_object_detection_plugin PRIVATE nx_kit nx_sdk nlohmann_json::nlohmann_json curl CppKafka::cppkafka)

target_compile_definitions(hailo_object_detection_plugin
    PRIVATE NX_PLUGIN_API=${API_EXPORT_MACRO}
)
