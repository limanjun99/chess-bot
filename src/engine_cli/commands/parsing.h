#pragma once

#include <cctype>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

namespace command::parsing {

// Splits a string into words with whitespace as a delimiter. Ignores extra spaces between words.
std::vector<std::string_view> split_string(std::string_view string);

// Returns a view to the first word (whitespace delimited) in a string, or an empty view if there are no words.
std::string_view first_word(std::string_view string);

// Parses a string into an integer of type `T`.
// Returns std::nullopt if given string is not an integer.
// Undefined behaviour if the integer value exceeds the representable range of `T`.
template <typename T>
  requires std::integral<T>
std::optional<T> parse_integer(std::string_view string);

}  // namespace command::parsing

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

template <typename T>
  requires std::integral<T>
std::optional<T> command::parsing::parse_integer(std::string_view string) {
  if (string.empty()) return std::nullopt;

  T value{0};
  const bool negative{string[0] == '-'};
  for (const char digit : string | std::views::drop(negative ? 1 : 0)) {
    if (!std::isdigit(digit)) return std::nullopt;
    value = value * 10 + (digit - '0');
  }
  if (negative) value = -value;

  return value;
}