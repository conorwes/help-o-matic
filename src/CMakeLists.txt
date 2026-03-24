cmake_minimum_required(VERSION 3.10)

# 1. Project name
project(HelpOMatic VERSION 1.0 LANGUAGES CXX)

# 2. Set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

# 3. Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 4. Collect all source files
# Note: Header files are included here so they show up in IDEs like Visual Studio
set(SOURCES
    main.cpp
    topic.cpp
    topic.hpp
    toc.cpp
    toc.hpp
    common_includes.hpp
    # Ensure you include the actual implementation of tinyxml2
    include/tinyxml2.cpp 
)

# 5. Define the Executable
add_executable(HelpOMatic ${SOURCES})

# 6. Set the name of the Executable
set_target_properties(HelpOMatic PROPERTIES OUTPUT_NAME "HelpFileSkeletonGenerator")

# 7. Include Directories
# This tells the compiler where to look for "include/tinyxml2.h"
target_include_directories(HelpOMatic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(HelpOMatic PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)