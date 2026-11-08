cmake_minimum_required(VERSION 3.20)
project(Lecture2PDF VERSION 0.1.0 LANGUAGES CXX)

add_library(lecture2pdf
    src/document/Slide.cpp
    src/document/Document.cpp
    src/storage/FileStorage.cpp
    src/pdf/PdfGenerator.cpp
    src/document/DocumentService.cpp
)
add_library(lecture2pdf::core ALIAS lecture2pdf)
target_compile_features(lecture2pdf PUBLIC cxx_std_17)
target_include_directories(lecture2pdf PUBLIC include)
if(MSVC)
    target_compile_options(lecture2pdf PRIVATE /W4 /utf-8)
else()
    target_compile_options(lecture2pdf PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(LECTURE2PDF_WITH_TESSERACT "Build PDF adapter and diagnostic CLI" OFF)
if(LECTURE2PDF_WITH_TESSERACT)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(TESSERACT REQUIRED IMPORTED_TARGET tesseract)
    pkg_check_modules(LEPTONICA REQUIRED IMPORTED_TARGET lept)
    add_library(lecture2pdf_tesseract src/pdf/TesseractPdfBackend.cpp)
    target_link_libraries(lecture2pdf_tesseract PUBLIC lecture2pdf PkgConfig::TESSERACT)
    add_executable(lecture2pdf_pdf_demo examples/pdf_demo.cpp)
    target_link_libraries(lecture2pdf_pdf_demo PRIVATE lecture2pdf_tesseract PkgConfig::LEPTONICA)
endif()

include(CTest)
if(BUILD_TESTING)
    find_package(GTest CONFIG QUIET)
    if(NOT GTest_FOUND)
        include(FetchContent)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            # GoogleTest v1.15.2, pinned to an immutable revision.
            GIT_TAG b514bdc898e2951020cbdca1304b75f5950d1f59
        )
        FetchContent_MakeAvailable(googletest)
    endif()
    add_executable(lecture2pdf_tests
        tests/DocumentTest.cpp tests/FileStorageTest.cpp tests/PdfGeneratorTest.cpp
        tests/DocumentServiceTest.cpp)
    target_link_libraries(lecture2pdf_tests PRIVATE lecture2pdf GTest::gtest_main)
    if(MSVC)
        target_compile_options(lecture2pdf_tests PRIVATE /utf-8)
    endif()
    include(GoogleTest)
    gtest_discover_tests(lecture2pdf_tests)
endif()
