#include "engine_cli.h"

#include <gtest/gtest.h>

#include <chrono>
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

// The EngineCliMoveTime test suite tests that a `go movetime` command is able to be read, processed, and
// responded to with a move within the given movetime.

void expect_engine_response_time(std::chrono::milliseconds movetime) {
  const std::string command{std::format("go movetime {}\n", movetime.count())};
  std::stringstream input_stream{command};
  std::stringstream output_stream{};

  EngineCli engine_cli{input_stream, output_stream};
  const auto start_time{std::chrono::steady_clock::now()};
  engine_cli.start();
  engine_cli.wait();
  std::string s;
  std::getline(output_stream, s);
  const auto end_time{std::chrono::steady_clock::now()};
  const std::chrono::duration<double, std::milli> time_taken{end_time - start_time};

  EXPECT_TRUE(s.starts_with("bestmove "));
  const std::string error_message{std::format("Took {} to respond to movetime of {}", time_taken, movetime)};
  EXPECT_TRUE(time_taken < movetime) << error_message;
}

TEST(EngineCliMoveTime, RespondsIn5ms) { expect_engine_response_time(std::chrono::milliseconds{5}); }
TEST(EngineCliMoveTime, RespondsIn10ms) { expect_engine_response_time(std::chrono::milliseconds{10}); }
TEST(EngineCliMoveTime, RespondsIn20ms) { expect_engine_response_time(std::chrono::milliseconds{20}); }
TEST(EngineCliMoveTime, RespondsIn50ms) { expect_engine_response_time(std::chrono::milliseconds{50}); }
TEST(EngineCliMoveTime, RespondsIn100ms) { expect_engine_response_time(std::chrono::milliseconds{100}); }
TEST(EngineCliMoveTime, RespondsIn500ms) { expect_engine_response_time(std::chrono::milliseconds{500}); }