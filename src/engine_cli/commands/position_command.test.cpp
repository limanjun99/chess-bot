#include "position_command.h"

#include <gtest/gtest.h>

#include <iostream>

TEST(PositionCommandParsing, ValidStartPos) {
  const auto command{PositionCommand::from_string("position startpos")};
  EXPECT_TRUE(command);
  EXPECT_EQ((*command)->get_position(), chess::Board::initial());
}

TEST(PositionCommandParsing, ValidFen) {
  const std::string input{"position fen rnbq1bnr/ppppkppp/8/4p3/4P3/8/PPPPKPPP/RNBQ1BNR w - - 2 3"};
  const auto command{PositionCommand::from_string(input)};
  const chess::Board expected_board{
      chess::Board::from_fen("rnbq1bnr/ppppkppp/8/4p3/4P3/8/PPPPKPPP/RNBQ1BNR w - - 2 3")};

  EXPECT_TRUE(command);
  EXPECT_EQ((*command)->get_position(), expected_board);
}

TEST(PositionCommandParsing, ValidStartPosWithMoves) {
  const std::string input{"position startpos moves e2e4 d7d5 e4d5"};
  const auto command{PositionCommand::from_string(input)};
  const chess::Board expected_board{
      chess::Board::from_fen("rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2")};

  EXPECT_TRUE(command);
  EXPECT_EQ((*command)->get_position(), expected_board);
}

TEST(PositionCommandParsing, ValidFenWithMoves) {
  const std::string input{
      "position fen r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3 moves g8f6 e1g1 f6e4"};
  const auto command{PositionCommand::from_string(input)};
  const chess::Board expected_board{
      chess::Board::from_fen("r1bqkb1r/pppp1ppp/2n5/4p3/2B1n3/5N2/PPPP1PPP/RNBQ1RK1 w kq - 0 5")};

  EXPECT_TRUE(command);
  EXPECT_EQ((*command)->get_position(), expected_board);
}

TEST(PositionCommand, HasCorrectUsageMessage) {
  EXPECT_EQ(PositionCommand::get_usage_info(),
            "Invalid usage of position command. "
            "Expected: position [fen <fenstring> | startpos ]  moves <move1> .... <movei>");
}