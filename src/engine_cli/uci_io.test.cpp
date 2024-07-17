#include "uci_io.h"

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

TEST(UciIO, InputDetectsEOF) {
  size_t input_lines_count{10};

  std::string input;
  for (size_t i{0}; i < input_lines_count; i++) input += "uci\n";

  std::stringstream input_stream{input};
  std::stringstream output_stream{};
  UciIO uci_io{input_stream, output_stream};

  for (size_t i{0}; i < input_lines_count; i++) {
    const auto command{uci_io.read()};
    EXPECT_EQ((*command)->get_input_string(), "uci");
  }
  const auto eof_command{uci_io.read()};
  EXPECT_EQ(eof_command.error().error_type, UciIO::ReadError::EndOfFile);
}