#include "uci_command.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(UciCommandParsing, ValidUci) {
  const auto command{UciCommand::from_string("uci")};
  EXPECT_TRUE(command);
}

TEST(UciCommandParsing, ErrorsOnExtraneousArguments) {
  const auto command{UciCommand::from_string("uci extra stuff")};
  EXPECT_FALSE(command);
}

TEST(UciCommand, HasCorrectUsageMessage) {
  EXPECT_EQ(UciCommand::get_usage_info(), "Invalid usage of uci command. Expected: uci");
}