#include "new_game_command.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(NewGameCommandParsing, ValidCommand) {
  const auto command{NewGameCommand::from_string("ucinewgame")};
  EXPECT_TRUE(command);
}

TEST(NewGameCommandParsing, ErrorsOnExtraneousArguments) {
  const auto command{NewGameCommand::from_string("ucinewgame extra stuff")};
  EXPECT_FALSE(command);
}

TEST(NewGameCommand, HasCorrectUsageMessage) {
  EXPECT_EQ(NewGameCommand::get_usage_info(), "Invalid usage of ucinewgame command. Expected: ucinewgame");
}