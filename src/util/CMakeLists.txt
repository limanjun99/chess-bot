add_library(
  util INTERFACE
  include/util/expected.h
)
target_compile_features(util INTERFACE cxx_std_23)
target_include_directories(util INTERFACE include)