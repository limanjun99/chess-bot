#pragma once

#include <expected>

namespace util {

// The make functions for expected / unexpected allows for efficient construction of a
// `std::expected<T, E>` without calling more move constructors / destructors of `T` and `E` than necessary.
// This is to mimic other std functions like `std::make_pair` or `std::make_optional` that do this.
// A non-negligible amount of time was spent googling for alternatives. If one is found, these functions below
// should be removed.

template <typename T, typename E>
class expected {
public:
  // Usage:
  // `using expected = util::expected<ResultType, ErrorType>;`
  // `return expected::make_unexpected(args);`
  static constexpr std::expected<T, E> make_expected(auto&&... args);

  // Usage:
  // `using expected = util::expected<ResultType, ErrorType>;`
  // `return expected::make_expected(args);`
  static constexpr std::expected<T, E> make_unexpected(auto&&... args);
};

}  // namespace util

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

template <typename T, typename E>
constexpr std::expected<T, E> util::expected<T, E>::make_expected(auto&&... args) {
  return std::expected<T, E>{std::in_place, std::forward<decltype(args)>(args)...};
}

template <typename T, typename E>
constexpr std::expected<T, E> util::expected<T, E>::make_unexpected(auto&&... args) {
  return std::expected<T, E>{std::unexpect, std::forward<decltype(args)>(args)...};
}