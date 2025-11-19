cmake_minimum_required(VERSION 3.10)
project(lab5)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror=maybe-uninitialized")

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.0
)
FetchContent_MakeAvailable(googletest)

add_executable(app ./main.cpp)

# enable_testing()
# add_executable(tests ./tests/tests.cpp)
# target_link_libraries(tests gtest_main)
# add_test(NAME Lab_5_Test COMMAND tests)