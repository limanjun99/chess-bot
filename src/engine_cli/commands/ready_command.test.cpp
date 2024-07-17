#include "ready_command.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(ReadyCommandParsing, Valid) {
  const auto command{ReadyCommand::from_string("isready")};
  EXPECT_TRUE(command);
}

TEST(ReadyCommandParsing, ErrorsOnExtraneousArguments) {
  const auto command{ReadyCommand::from_string("isready extra stuff")};
  EXPECT_FALSE(command);
}

TEST(ReadyCommandParsing, HasCorrectUsageMessage) {
  EXPECT_EQ(ReadyCommand::get_usage_info(), "Invalid usage of isready command. Expected: isready");
}