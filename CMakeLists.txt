cmake_minimum_required(VERSION 3.16)

project(Lab_1_Shavaliev LANGUAGES CXX)

add_executable(lab1
    src/main.cpp
)

target_compile_features(lab1 PRIVATE cxx_std_20)

set_target_properties(lab1 PROPERTIES
    CXX_EXTENSIONS OFF
)

target_include_directories(lab1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/sequences/SequenceArray
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/sequences
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/utils
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(lab1 PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

enable_testing()
add_subdirectory(test/googletest)

add_executable(avl_tests test/AVLTree_test.cpp)
target_compile_features(avl_tests PRIVATE cxx_std_20)

target_include_directories(avl_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/sequences/SequenceArray
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/sequences
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/include/Lab_2_2026/src/utils
)

target_link_libraries(avl_tests PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(avl_tests)