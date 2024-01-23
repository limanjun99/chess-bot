add_executable(benchmarks main.cpp board.cpp engine.cpp perft.cpp)

target_link_libraries(benchmarks PRIVATE chess)
target_link_libraries(benchmarks PRIVATE chess_engine)

target_compile_features(benchmarks PRIVATE cxx_std_20)
target_compile_options(benchmarks PRIVATE -Wall -Wextra -Wpedantic)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571)

FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG 344117638c8ff7e239044fd0fa7085839fc03021)

FetchContent_MakeAvailable(googletest googlebenchmark)
target_link_libraries(benchmarks PRIVATE benchmark::benchmark)
