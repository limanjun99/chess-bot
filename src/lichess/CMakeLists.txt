add_executable(lichess_bot config.cpp logger.cpp main.cpp)

target_compile_features(lichess_bot PRIVATE cxx_std_20)

target_link_libraries(lichess_bot PUBLIC chess)
target_link_libraries(lichess_bot PUBLIC chess_engine)
