add_library(
  chess_engine_nn
  src/chess_game.cpp
  src/engine.cpp
  src/model.cpp
  src/trainer.cpp
)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

target_include_directories(chess_engine_nn PRIVATE include/chess_engine_nn)
target_include_directories(chess_engine_nn PUBLIC include)

target_link_libraries(chess_engine_nn PRIVATE "${TORCH_LIBRARIES}")
include_directories(SYSTEM ${TORCH_INCLUDE_DIRS})
target_link_libraries(chess_engine_nn PUBLIC chess)

target_compile_features(chess_engine_nn PRIVATE cxx_std_20)
target_compile_options(chess_engine_nn PRIVATE -Wall -Wextra -Wpedantic -O3)


add_executable(
  chess_engine_nn_train
  src/main.cpp
)
target_link_libraries(chess_engine_nn_train PRIVATE chess_engine_nn)
target_compile_features(chess_engine_nn_train PRIVATE cxx_std_20)
target_compile_options(chess_engine_nn_train PRIVATE -Wall -Wextra -Wpedantic -O3)