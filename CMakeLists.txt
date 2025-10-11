cmake_minimum_required(VERSION 3.14)
project(custom_hashmap_bench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer -Ofast")

set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)

include(FetchContent)

# Google Benchmark
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)
FetchContent_MakeAvailable(benchmark)
add_executable(bench src/bench.cc)
add_executable(nobench src/nogooglebench.cc)
target_link_libraries(bench PRIVATE benchmark::benchmark gtest gtest_main)
