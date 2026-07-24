cmake_minimum_required(VERSION 3.21)

project(HelpOMatic VERSION 1.0.0 LANGUAGES CXX)

set(HELPOMATIC_RUNTIME_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bin")
file(REAL_PATH "${HELPOMATIC_RUNTIME_OUTPUT_DIR}" HELPOMATIC_RUNTIME_OUTPUT_DIR)

option(HELPOMATIC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# 4. Collect all source files
# Note: Header files are included here so they show up in IDEs like Visual Studio
set(SOURCES
    main.cpp
    help_map_parser.cpp
    help_map_parser.hpp
    topic.cpp
    topic.hpp
    toc.cpp
    toc.hpp
    common_includes.hpp
    # Ensure you include the actual implementation of tinyxml2
    include/tinyxml2.cpp 
)

add_executable(HelpOMatic ${SOURCES})

set_target_properties(HelpOMatic PROPERTIES
    OUTPUT_NAME "HelpFileSkeletonGenerator"
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${HELPOMATIC_RUNTIME_OUTPUT_DIR}"
)

target_compile_features(HelpOMatic PRIVATE cxx_std_20)

target_include_directories(HelpOMatic PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

function(helpomatic_apply_warnings target_name)
    if(MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /permissive- /Zc:__cplusplus)
        if(HELPOMATIC_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        if(HELPOMATIC_WARNINGS_AS_ERRORS)
            target_compile_options(${target_name} PRIVATE -Werror)
        endif()
    endif()
endfunction()

helpomatic_apply_warnings(HelpOMatic)

enable_testing()

add_executable(HelpOMaticParserSmoke
    tests/parser_smoke_test.cpp
    help_map_parser.cpp
    help_map_parser.hpp
    common_includes.hpp
    include/tinyxml2.cpp
)

target_compile_features(HelpOMaticParserSmoke PRIVATE cxx_std_20)
target_include_directories(HelpOMaticParserSmoke PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
helpomatic_apply_warnings(HelpOMaticParserSmoke)

add_executable(HelpOMaticOutputSmoke tests/output_smoke_test.cpp)
target_compile_features(HelpOMaticOutputSmoke PRIVATE cxx_std_20)
helpomatic_apply_warnings(HelpOMaticOutputSmoke)

add_test(
    NAME parser_smoke
    COMMAND HelpOMaticParserSmoke "${CMAKE_CURRENT_SOURCE_DIR}/../test_files/ExtraHelpObjectMap_Nanosecond.xml"
)

add_test(
    NAME generator_smoke
    COMMAND $<TARGET_FILE:HelpOMatic> "${CMAKE_CURRENT_SOURCE_DIR}/../test_files/ExtraHelpObjectMap_Nanosecond.xml"
)
set_tests_properties(generator_smoke PROPERTIES WORKING_DIRECTORY $<TARGET_FILE_DIR:HelpOMatic>)

add_test(NAME output_smoke COMMAND HelpOMaticOutputSmoke)
set_tests_properties(output_smoke PROPERTIES
    WORKING_DIRECTORY $<TARGET_FILE_DIR:HelpOMatic>
    DEPENDS generator_smoke
)