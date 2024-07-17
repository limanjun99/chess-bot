#include "engine_cli.h"

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

TEST(EngineCli, RespondsToUciCommand) {
  std::stringstream input_stream{"uci\n"};
  std::stringstream output_stream{};
  EngineCli engine_cli{input_stream, output_stream};
  engine_cli.start();

  std::string s;
  std::getline(output_stream, s);
  EXPECT_EQ(s, "id name placeholderName");
  std::getline(output_stream, s);
  EXPECT_EQ(s, "id author placeholderAuthor");
  std::getline(output_stream, s);
  EXPECT_EQ(s, "uciok");
}