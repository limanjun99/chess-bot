#include "quit_command.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(QuitCommandParsing, Valid) {
  const auto command{QuitCommand::from_string("quit")};
  EXPECT_TRUE(command);
}

TEST(QuitCommandParsing, ErrorsOnExtraneousArguments) {
  const auto command{QuitCommand::from_string("quit extra stuff")};
  EXPECT_FALSE(command);
}

TEST(QuitCommand, HasCorrectUsageMessage) {
  EXPECT_EQ(QuitCommand::get_usage_info(), "Invalid usage of quit command. Expected: quit");
}