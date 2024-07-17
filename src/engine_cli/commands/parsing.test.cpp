#include "parsing.h"

#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

TEST(CommandParsingSplitString, EmptyString) {
  const std::string string{""};
  const auto words{command::parsing::split_string(string)};
  EXPECT_EQ(words.size(), 0);
}

TEST(CommandParsingSplitString, FullyWhitespace) {
  const std::string string{" \n\t "};
  const auto words{command::parsing::split_string(string)};
  EXPECT_EQ(words.size(), 0);
}

TEST(CommandParsingSplitString, SingleWordString) {
  const std::string string{"  uci   "};
  const auto words{command::parsing::split_string(string)};
  EXPECT_EQ(words.size(), 1);
  EXPECT_EQ(words[0], "uci");
}

TEST(CommandParsingSplitString, MultiWordString) {
  const std::string string{"position  some fen string  moves  move1   move2 "};
  const auto words{command::parsing::split_string(string)};
  const std::array expected_words{"position", "some", "fen", "string", "moves", "move1", "move2"};
  EXPECT_EQ(words.size(), expected_words.size());
  for (size_t i{0}; i < expected_words.size(); i++) {
    EXPECT_EQ(words[i], expected_words[i]);
  }
}

TEST(CommandParsingInteger, Zero) {
  const auto integer_result{command::parsing::parse_integer<int>("0")};
  EXPECT_EQ(*integer_result, 0);
}

TEST(CommandParsingInteger, NegativeOne) {
  const auto integer_result{command::parsing::parse_integer<int>("-1")};
  EXPECT_EQ(*integer_result, -1);
}

TEST(CommandParsingInteger, One) {
  const auto integer_result{command::parsing::parse_integer<int>("1")};
  EXPECT_EQ(*integer_result, 1);
}

TEST(CommandParsingInteger, LargeNumber) {
  const auto integer_result{command::parsing::parse_integer<int64_t>("12345678987654321")};
  EXPECT_EQ(*integer_result, 12345678987654321);
}

TEST(CommandParsingInteger, LargeNegativeNumber) {
  const auto integer_result{command::parsing::parse_integer<int64_t>("-98765432111111111")};
  EXPECT_EQ(*integer_result, -98765432111111111);
}

TEST(CommandParsingInteger, FailsOnScientificNotation) {
  const auto integer_result{command::parsing::parse_integer<int>("1e5")};
  EXPECT_FALSE(integer_result);
}

TEST(CommandParsingInteger, FailsOnDecimals) {
  const auto integer_result{command::parsing::parse_integer<int>("12.34")};
  EXPECT_FALSE(integer_result);
}

TEST(CommandParsingInteger, FailsOnGarbage) {
  const auto integer_result{command::parsing::parse_integer<int>("123a")};
  EXPECT_FALSE(integer_result);
}

TEST(CommandParsingFirstWord, SingleWord) {
  const auto word{command::parsing::first_word(" single")};
  EXPECT_EQ(word, "single");
}

TEST(CommandParsingFirstWord, MultipleWords) {
  const auto word{command::parsing::first_word("multiple words here")};
  EXPECT_EQ(word, "multiple");
}

TEST(CommandParsingFirstWord, NoWords) {
  const auto word{command::parsing::first_word(" \t\n  ")};
  EXPECT_EQ(word, "");
}