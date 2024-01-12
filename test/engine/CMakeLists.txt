add_executable(chess_engine_tests main.cpp ab_engine_test.cpp)

target_link_libraries(chess_engine_tests PRIVATE chess)
target_link_libraries(chess_engine_tests PRIVATE chess_engine)
target_include_directories(chess_engine_tests PRIVATE ${DOCTEST_INCLUDE_DIR})

target_compile_features(chess_engine_tests PRIVATE cxx_std_20)
target_compile_options(chess_engine_tests PRIVATE -Wall -Wextra -Wpedantic)
