add_library(chess src/bitboard.cpp src/board.cpp src/move.cpp src/move_gen.cpp src/piece.cpp src/player.cpp)

target_compile_features(chess PRIVATE cxx_std_20)
target_include_directories(chess PRIVATE include/chess)
target_include_directories(chess PUBLIC include)
target_compile_options(chess PRIVATE -Wall -Wextra -Wpedantic -O3)

add_executable(chess_magic_calculator tools/magic_calculator.cpp)

target_compile_features(chess_magic_calculator PRIVATE cxx_std_20)
target_compile_options(chess_magic_calculator PRIVATE -Wall -Wextra -Wpedantic
                                                      -O3)
