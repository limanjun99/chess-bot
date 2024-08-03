#include "file.h"

#include <format>
#include <ranges>
#include <stdexcept>

std::ifstream file::open_read(const std::filesystem::path& file_path) {
  auto file = std::ifstream{file_path};
  if (file) return file;

  const auto error_message = std::format("Failed to open csv file for reading at: {}", file_path.string());
  throw std::runtime_error{error_message};
}

std::ofstream file::open_write(const std::filesystem::path& file_path) {
  auto file = std::ofstream{file_path};
  if (file) return file;

  const auto error_message = std::format("Failed to open csv file for writing at: {}", file_path.string());
  throw std::runtime_error{error_message};
}

void file::csv::check_header(std::string_view header, std::string_view expected_header,
                             const std::filesystem::path& file_path) {
  if (header == expected_header) return;

  const auto error_message = std::format(
      "Invalid header in csv file {}:\n"
      "  Expected: {}\n"
      "  Found: {}",
      file_path.string(), expected_header, header);
  throw std::runtime_error{error_message};
}

std::vector<std::string_view> file::csv::split_line(std::string_view line, int32_t column_count,
                                                    const std::filesystem::path& file_path) {
  auto values = line | std::views::split(',') |
                std::views::transform([](auto&& range) { return std::string_view{range}; }) |
                std::ranges::to<std::vector>();

  if (static_cast<int32_t>(values.size()) != column_count) {
    const auto error_message = std::format(
        "Invalid line in csv file {} (expected {} columns, but got {} columns):\n"
        "  {}",
        file_path.string(), column_count, values.size(), line);
    throw std::runtime_error{error_message};
  }

  return values;
}