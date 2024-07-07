add_executable(benchmarks main.cpp board.cpp engine.cpp perft.cpp)

target_link_libraries(benchmarks PRIVATE chess_engine)
target_link_libraries(benchmarks PRIVATE chess)
target_link_libraries(benchmarks PRIVATE doctest::doctest)

target_compile_features(benchmarks PRIVATE cxx_std_20)
target_compile_options(benchmarks PRIVATE -Wall -Wextra -Wpedantic)

find_package(benchmark CONFIG REQUIRED)
target_link_libraries(benchmarks PRIVATE benchmark::benchmark)
