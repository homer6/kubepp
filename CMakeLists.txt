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

include_directories(
    "/usr/local/include/kubernetes/include"
    "/usr/local/include/kubernetes/config"
    "/usr/local/include/kubernetes/api"
)

# Add main application
add_executable(kubepp src/main.cpp ${SOURCES})

# Find the fmt library
find_package(fmt REQUIRED)
target_link_libraries(kubepp PRIVATE kubernetes fmt::fmt)


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







# # Enable testing with CTest
# enable_testing()

# # Find Google Test package (should work if GTest is installed properly)
# find_package(GTest REQUIRED)
# include_directories(${GTEST_INCLUDE_DIRS})


# # Add test executable
# add_executable(test_kubepp tests/TestKubeppApp.cpp ${SOURCES})
# target_link_libraries(test_kubepp ${GTEST_LIBRARIES} pthread fmt::fmt kubernetes)


# # Add test to the project to be run with 'make test' or 'ctest'
# add_test(NAME kubepp_test COMMAND test_kubepp)




install(TARGETS kubepp DESTINATION bin)

add_library(kubepp_lib SHARED ${SOURCES})
target_link_libraries(kubepp_lib PRIVATE kubernetes fmt::fmt)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/src/"
        DESTINATION "include/kubepp"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp")


install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/kubepp_libConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/kubepp_libConfigVersion.cmake"
  DESTINATION lib/cmake/kubepp_lib
)


install(TARGETS kubepp_lib
    EXPORT kubepp_libTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(EXPORT kubepp_libTargets
    FILE kubepp_libTargets.cmake
    NAMESPACE kubepp_lib::
    DESTINATION lib/cmake/kubepp_lib
)
