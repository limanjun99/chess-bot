#include "stop_command.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(StopCommandParsing, Valid) {
  const auto command{StopCommand::from_string("stop")};
  EXPECT_TRUE(command);
}

TEST(StopCommandParsing, ErrorsOnExtraneousArguments) {
  const auto command{StopCommand::from_string("stop extra stuff")};
  EXPECT_FALSE(command);
}

TEST(StopCommandParsing, HasCorrectUsageMessage) {
  EXPECT_EQ(StopCommand::get_usage_info(), "Invalid usage of stop command. Expected: stop");
}