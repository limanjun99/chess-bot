#include "util.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>

class CopylessAndMoveless {
public:
  explicit CopylessAndMoveless(int){};
  CopylessAndMoveless(const CopylessAndMoveless&) = delete;
  CopylessAndMoveless(CopylessAndMoveless&&) = delete;
  CopylessAndMoveless& operator=(const CopylessAndMoveless&) = delete;
  CopylessAndMoveless& operator=(CopylessAndMoveless&&) = delete;
  ~CopylessAndMoveless() = default;
};

TEST(UtilMakeExpected, DoesNotRequireMovesOrCopies) {
  const auto func{[]() -> std::expected<CopylessAndMoveless, std::string> {
    return util::expected<CopylessAndMoveless, std::string>::make_expected(1);
  }};
  const auto x{func()};
  SUCCEED();
}

TEST(UtilMakeUnexpected, DoesNotRequireMovesOrCopies) {
  const auto func{[]() -> std::expected<std::string, CopylessAndMoveless> {
    return util::expected<std::string, CopylessAndMoveless>::make_unexpected(1);
  }};
  const auto x{func()};
  SUCCEED();
}