#pragma once

#include <expected>
#include <format>
#include <memory>
#include <string>
#include <string_view>

#include "parsing.h"
#include "util/expected.h"

namespace command::detail {

// clang-format off
template <typename T>
concept IsParameterlessCommand = requires {
  { T::get_name() } -> std::convertible_to<std::string_view>;
};
// clang-format on

// Implement helper functions for parameterless commands.
template <typename Derived>
class ParameterlessCommand {
public:
  // Constructs a Command from an input string, or returns an error string if the input is invalid.
  [[nodiscard]] static std::expected<std::unique_ptr<Derived>, std::string> from_string(std::string input_string);

  // Returns a string explaining how to use this command.
  [[nodiscard]] static std::string_view get_usage_info();
};

}  // namespace command::detail

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

namespace command::detail {

template <typename Derived>
std::expected<std::unique_ptr<Derived>, std::string> detail::ParameterlessCommand<Derived>::from_string(
    std::string input_string) {
  // Derived is incomplete before this, hence `requires` cannot be used instead.
  static_assert(IsParameterlessCommand<Derived>);

  using expected = util::expected<std::unique_ptr<Derived>, std::string>;

  const auto words{command::parsing::split_string(input_string)};
  const auto& expected_name{Derived::get_name()};

  const bool valid{words.size() == 1 && words[0] == expected_name};
  if (!valid) {
    return expected::make_unexpected(std::format("Invalid usage of {0} command. Expected: {0}", expected_name));
  }

  // Using `new` to access private constructor.
  return expected::make_expected(std::unique_ptr<Derived>{new Derived(std::move(input_string))});
}

template <typename Derived>
std::string_view detail::ParameterlessCommand<Derived>::get_usage_info() {
  // Derived is incomplete before this, hence `requires` cannot be used instead.
  static_assert(IsParameterlessCommand<Derived>);

  static const std::string usage_info{std::format("Invalid usage of {0} command. Expected: {0}", Derived::get_name())};
  return usage_info;
}

}  // namespace command::detail