add_library(
  chess
  src/board.cpp
  src/move_gen.cpp
  src/pieces/bishop.cpp
  src/pieces/king.cpp
  src/pieces/knight.cpp
  src/pieces/rook.cpp
  src/pieces/queen.cpp)

target_compile_features(chess PRIVATE cxx_std_20)
target_include_directories(chess PRIVATE include/chess)
target_include_directories(chess PUBLIC include)
target_compile_options(chess PRIVATE -Wall -Wextra -O3)

add_executable(chess_magic_calculator tools/magic_calculator.cpp)

target_compile_features(chess_magic_calculator PRIVATE cxx_std_20)
target_compile_options(chess_magic_calculator PRIVATE -Wall -Wextra -O3)
