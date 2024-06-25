add_executable(chess_tests main.cpp board.cpp perft.cpp uci.cpp)

target_link_libraries(chess_tests PRIVATE chess)
target_include_directories(chess_tests PRIVATE ${DOCTEST_INCLUDE_DIR})

target_compile_features(chess_tests PRIVATE cxx_std_20)
target_compile_options(chess_tests PRIVATE -Wall -Wextra -Wpedantic)
