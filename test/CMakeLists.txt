include(FetchContent)
FetchContent_Declare(
  doctest
  URL https://raw.githubusercontent.com/doctest/doctest/v2.4.11/doctest/doctest.h
  DOWNLOAD_NO_EXTRACT TRUE)
FetchContent_MakeAvailable(doctest)
set(DOCTEST_INCLUDE_DIR ${doctest_SOURCE_DIR})

add_subdirectory(chess)
