cmake_minimum_required(VERSION 3.10)
project(kubepp VERSION 0.1)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories for your project
include_directories("${PROJECT_SOURCE_DIR}/src")

# Set all of the sources
set(SOURCES
    src/KubeppApp.cpp
)


# Add main application
add_executable(kubepp src/main.cpp ${SOURCES})

# Enable testing with CTest
enable_testing()

# Find Google Test package (should work if GTest is installed properly)
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Find the fmt library
find_package(fmt REQUIRED)
target_link_libraries(kubepp fmt::fmt)



# Add test executable
#add_executable(test_kubepp tests/TestKubeppApp.cpp ${SOURCES})
#target_link_libraries(test_kubepp ${GTEST_LIBRARIES} pthread fmt::fmt)


# Add test to the project to be run with 'make test' or 'ctest'
#add_test(NAME kubepp_test COMMAND test_kubepp)


