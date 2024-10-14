cmake_minimum_required(VERSION 3.10)
project(kubepp VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set the build type to Debug if it's not explicitly set by the user
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Include directories for your project
include_directories("${PROJECT_SOURCE_DIR}/src")

# Set all of the sources
set(SOURCES
    src/KubeppApp.cpp
    src/KubernetesClient.cpp
    src/ResourceDescription.cpp
    src/Query.cpp
    src/cjson.cpp
)

# Platform-specific settings
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # macOS specific settings
    include_directories("/usr/local/include")
    link_directories("/usr/local/lib")
    find_package(spdlog REQUIRED)
    find_package(fmt REQUIRED)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # Ubuntu specific settings
    find_package(spdlog REQUIRED CONFIG)
    find_package(fmt REQUIRED CONFIG)
    if(NOT TARGET spdlog::spdlog)
        message(FATAL_ERROR "spdlog target not found")
    endif()
    if(NOT TARGET fmt::fmt)
        message(FATAL_ERROR "fmt target not found")
    endif()
endif()

# Add include directories for fmt and spdlog
include_directories(${spdlog_INCLUDE_DIRS} ${fmt_INCLUDE_DIRS})

# Ensure Kubernetes headers are found
include_directories(
    "/usr/local/include/kubernetes"
    "/usr/local/include/kubernetes/api"
    "/usr/local/include/kubernetes/config"
    "/usr/local/include/kubernetes/external"
    "/usr/local/include/kubernetes/include"
    "/usr/local/include/kubernetes/model"
    "/usr/local/include/kubernetes/watch"
    "/usr/local/include/kubernetes/websocket"
)

# Add main application
add_executable(kubepp src/main.cpp ${SOURCES})

# Link libraries for the main application
target_link_libraries(kubepp PRIVATE kubernetes fmt::fmt spdlog::spdlog)

# Add shared library
add_library(kubepp_lib SHARED ${SOURCES})
target_link_libraries(kubepp_lib PRIVATE kubernetes fmt::fmt spdlog::spdlog)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/kubepp_libConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/kubepp_libConfig.cmake"
  INSTALL_DESTINATION lib/cmake/kubepp_lib
)

# Install the main executable
install(TARGETS kubepp DESTINATION bin)

# Install headers
install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/"
        DESTINATION "include/kubepp"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp")

# Install CMake configuration files
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/kubepp_libConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/kubepp_libConfigVersion.cmake"
  DESTINATION lib/cmake/kubepp_lib
)

# Install the shared library
install(TARGETS kubepp_lib
    EXPORT kubepp_libTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Export library targets
install(EXPORT kubepp_libTargets
    FILE kubepp_libTargets.cmake
    NAMESPACE kubepp_lib::
    DESTINATION lib/cmake/kubepp_lib
)
