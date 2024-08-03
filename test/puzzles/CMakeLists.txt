# Puzzle Testing

add_executable(
  puzzles
  src/config.cpp
  src/data.cpp
  src/file.cpp
  src/main.cpp
  src/puzzle.cpp
  src/solve.cpp
  src/statistic.cpp
)

target_link_libraries(puzzles PRIVATE chess_engine)
target_link_libraries(puzzles PRIVATE chess)

target_compile_features(puzzles PRIVATE cxx_std_23)
target_compile_options(puzzles PRIVATE -Wall -Wextra -O3)
set_target_properties(puzzles PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${g_test_output_directory})