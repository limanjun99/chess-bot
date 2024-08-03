#pragma once

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>

namespace file {

// Opens a file for reading. Throws a `runtime_error` if the file failed to open.
std::ifstream open_read(const std::filesystem::path& file_path);

// Opens a file for writing. Throws a `runtime_error` if the file failed to open.
std::ofstream open_write(const std::filesystem::path& file_path);

}  // namespace file

namespace file::csv {

// Check the header of a csv file. Throws a `runtime_error` if the header is invalid.
void check_header(std::string_view header, std::string_view expected_header, const std::filesystem::path& file_path);

// Splits a line in the csv file into its values.
// If the line does not have the correct number of columns, a `runtime_error` is thrown.
// This function assumes that all commas are used as delimiters.
std::vector<std::string_view> split_line(std::string_view line, int32_t column_count,
                                         const std::filesystem::path& file_path);

// Parses a base-10, non-negative integer and returns it.
template <std::integral T>
T parse_non_negative_integer(std::string_view string);

}  // namespace file::csv

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

template <std::integral T>
T file::csv::parse_non_negative_integer(std::string_view string) {
  T value{0};
  for (const auto& digit : string) {
    value = value * 10 + (digit - '0');
  }
  return value;
}