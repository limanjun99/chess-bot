#include "uci_io.h"

#include <string>

#include "util.h"

UciIO::UciIO(std::istream& input_stream, std::ostream& output_stream)
    : input_stream{input_stream}, output_stream{output_stream} {}

std::expected<std::unique_ptr<Command>, UciIO::ReadError> UciIO::read() {
  using expected = util::expected<std::unique_ptr<Command>, ReadError>;

  std::string input;
  if (!std::getline(input_stream, input)) {
    return expected::make_unexpected(ReadError{ReadError::EndOfFile, ""});
  }

  auto command_result{command::from_string(std::move(input))};
  if (!command_result) {
    return expected::make_unexpected(ReadError{ReadError::InvalidCommand, std::move(command_result.error())});
  }

  return expected::make_expected(std::move(*command_result));
}

void UciIO::write(const Output& output) { output_stream << output << std::endl; }