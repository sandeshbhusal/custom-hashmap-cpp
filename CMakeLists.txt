cmake_minimum_required(VERSION 4.1)
project(custom_hashmap_bench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer -Ofast")
set(CMAKE_GENERATOR Ninja)

include(FetchContent)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.4
)
FetchContent_MakeAvailable(benchmark)

add_executable(bench src/bench.cc)
add_executable(nobench src/nogooglebench.cc)

target_link_libraries(bench PRIVATE benchmark::benchmark)
