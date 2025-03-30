#cmake_minimum_required(VERSION 3.28)
#include(FetchContent)

option(ENABLE_TESTING "Enable building tests" ON)


project(VuTest)
add_executable(Google_Tests_run Test1.cpp)

if(ENABLE_TESTING)
    enable_testing()
endif()

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest
        GIT_TAG v1.16.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(googletest)
target_link_libraries(Google_Tests_run PRIVATE gtest gtest_main VuLibs)