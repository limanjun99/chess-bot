#include "best_move_output.h"

#include <format>

BestMoveOutput::BestMoveOutput(chess::Move move) : move{move} {}

std::string BestMoveOutput::to_string() const { return std::format("bestmove {}", move.to_uci()); }