#include "go_command.h"

#include <gtest/gtest.h>

#include <iostream>
#include <string>

TEST(GoCommandParsing, ValidOptions) {
  const auto command{
      GoCommand::from_string("go wtime 1000 btime 1000 winc 1 binc 1 depth 1 nodes 1 movetime 1 infinite")};
  EXPECT_TRUE(command);
}

TEST(GoCommandParsing, InvalidArgumentToOptions) {
  const auto command{GoCommand::from_string("go wtime ? btime ?")};
  EXPECT_EQ(command.error(), "Invalid argument '?' for option 'wtime' of go command. Expected: wtime <integer>");
}

TEST(GoCommandParsing, MissingArgumentForOption) {
  const auto command{GoCommand::from_string("go nodes")};
  EXPECT_EQ(command.error(), "Missing argument for option 'nodes' of go command. Expected: nodes <integer>");
}