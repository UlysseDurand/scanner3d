cmake_minimum_required(VERSION 3.10)
project(Scanner3Dv1)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV package
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

# Include directories
include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIR}
    src/
)

add_subdirectory(external/CLI11)

# Add executable
add_executable(scanner3d
    src/main.cpp
    src/MC.h
)

# Link libraries
target_link_libraries(scanner3d
    ${OpenCV_LIBS}
    CLI11::CLI11
)
target_include_directories(scanner3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external)


# Copy data directory to build directory
file(COPY data/ DESTINATION ${CMAKE_BINARY_DIR}/data)
