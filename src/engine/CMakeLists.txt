add_library(chess_engine src/naive_engine.cpp)

target_compile_features(chess_engine PRIVATE cxx_std_20)
target_include_directories(chess_engine PRIVATE include/chess_engine)
target_include_directories(chess_engine PUBLIC include)

target_link_libraries(chess_engine PUBLIC chess)
