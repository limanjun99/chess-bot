add_executable(chess_tests main.cpp board.cpp perft.cpp uci.cpp)

target_link_libraries(chess_tests PRIVATE chess)
target_link_libraries(chess_tests PRIVATE doctest::doctest)

target_compile_features(chess_tests PRIVATE cxx_std_20)
target_compile_options(chess_tests PRIVATE -Wall -Wextra -Wpedantic)
