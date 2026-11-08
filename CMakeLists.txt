cmake_minimum_required(VERSION 3.20)
project(Lecture2PDF VERSION 0.1.0 LANGUAGES CXX)

add_library(lecture2pdf
    src/document/Slide.cpp
    src/document/Document.cpp
    src/storage/FileStorage.cpp
)
add_library(lecture2pdf::core ALIAS lecture2pdf)
target_compile_features(lecture2pdf PUBLIC cxx_std_17)
target_include_directories(lecture2pdf PUBLIC include)
if(MSVC)
    target_compile_options(lecture2pdf PRIVATE /W4 /utf-8)
else()
    target_compile_options(lecture2pdf PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
            GIT_SHALLOW TRUE
        )
        FetchContent_MakeAvailable(googletest)
    endif()
    add_executable(lecture2pdf_tests tests/DocumentTest.cpp tests/FileStorageTest.cpp)
    target_link_libraries(lecture2pdf_tests PRIVATE lecture2pdf GTest::gtest_main)
    if(MSVC)
        target_compile_options(lecture2pdf_tests PRIVATE /utf-8)
    endif()
    include(GoogleTest)
    gtest_discover_tests(lecture2pdf_tests)
endif()
