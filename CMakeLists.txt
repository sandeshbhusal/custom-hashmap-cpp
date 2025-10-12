cmake_minimum_required(VERSION 4.1)
project(custom_hashmap_bench LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fno-omit-frame-pointer -Ofast")

include(FetchContent)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG        v1.9.4
)
FetchContent_MakeAvailable(benchmark)

# FetchContent_Declare(
#     googletest
#     GIT_REPOSITORY https://github.com/google/googletest.git
#     GIT_TAG        1.17.0
# )
# 
# FetchContent_MakeAvailable(benchmark googletest)

add_executable(bench src/bench.cc)
add_executable(nobench src/nogooglebench.cc)

target_link_libraries(bench PRIVATE benchmark::benchmark)
