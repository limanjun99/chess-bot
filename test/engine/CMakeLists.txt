add_executable(chess_engine_tests main.cpp engine.cpp)

target_link_libraries(chess_engine_tests PRIVATE chess_engine)
target_link_libraries(chess_engine_tests PRIVATE chess)
target_link_libraries(chess_engine_tests PRIVATE doctest::doctest)

target_compile_features(chess_engine_tests PRIVATE cxx_std_20)
target_compile_options(chess_engine_tests PRIVATE -Wall -Wextra -Wpedantic)
