#include "debug_command.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>

TEST(DebugCommandParsing, ValidDebugOn) {
  const auto command{DebugCommand::from_string("debug on")};
  EXPECT_TRUE(command);
  EXPECT_TRUE((*command)->get_debug_mode());
}

TEST(DebugCommandParsing, ValidDebugOff) {
  const auto command{DebugCommand::from_string("debug off")};
  EXPECT_TRUE(command);
  EXPECT_FALSE((*command)->get_debug_mode());
}

TEST(DebugCommandParsing, ErrorsOnIncompleteDebugCommand) {
  const auto command{DebugCommand::from_string("debug")};
  EXPECT_FALSE(command);
  EXPECT_EQ(command.error(), DebugCommand::get_usage_info());
}

TEST(DebugCommandParsing, ErrorsOnInvalidDebugMode) {
  const auto command{DebugCommand::from_string("debug xyz")};
  EXPECT_FALSE(command);
  EXPECT_EQ(command.error(), DebugCommand::get_usage_info());
}

TEST(DebugCommandParsing, ErrorsOnExtraneousArguments) {
  const auto command{DebugCommand::from_string("debug on irrelevantstuff")};
  EXPECT_FALSE(command);
  EXPECT_EQ(command.error(), DebugCommand::get_usage_info());
}

TEST(DebugCommand, HasCorrectUsageMessage) {
  EXPECT_EQ(DebugCommand::get_usage_info(), "Invalid usage of debug command. Expected: debug [on | off]");
}