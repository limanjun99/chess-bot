#include "naive_engine.h"

#include <limits.h>

#include "chess/moveset.h"

namespace evaluation {
constexpr int LOSING = INT_MIN;
constexpr int WINNING = INT_MAX;
constexpr int DRAW = 0;
constexpr int bishop = 3;
constexpr int knight = 3;
constexpr int pawn = 1;
constexpr int queen = 9;
constexpr int rook = 5;
}  // namespace evaluation

constexpr int QUIESCENCE_SEARCH_DEPTH = 5;

NaiveEngine::NaiveEngine(int depth) : depth{depth} {}

Move NaiveEngine::make_move(const Board& board) {
  MoveSet move_set = board.generate_moves();
  int board_evaluation = evaluation::LOSING;
  std::optional<Move> best_move = std::nullopt;
  while (auto result = move_set.apply_next()) {
    auto& [move, new_board] = *result;
    int new_board_evaluation = search(new_board, 0);
    if (-new_board_evaluation >= board_evaluation) {
      board_evaluation = -new_board_evaluation;
      best_move = std::optional(move);
    }
  }
  return *best_move;
}

int NaiveEngine::evaluate_board(const Board& board) {
  int score = 0;
  const Player& cur_player = board.cur_player();
  const Player& opp_player = board.opp_player();
  score += bitboard::count(cur_player.get_bitboard(Piece::Bishop)) * evaluation::bishop;
  score += bitboard::count(cur_player.get_bitboard(Piece::Knight)) * evaluation::knight;
  score += bitboard::count(cur_player.get_bitboard(Piece::Pawn)) * evaluation::pawn;
  score += bitboard::count(cur_player.get_bitboard(Piece::Queen)) * evaluation::queen;
  score += bitboard::count(cur_player.get_bitboard(Piece::Rook)) * evaluation::rook;
  score -= bitboard::count(opp_player.get_bitboard(Piece::Bishop)) * evaluation::bishop;
  score -= bitboard::count(opp_player.get_bitboard(Piece::Knight)) * evaluation::knight;
  score -= bitboard::count(opp_player.get_bitboard(Piece::Pawn)) * evaluation::pawn;
  score -= bitboard::count(opp_player.get_bitboard(Piece::Queen)) * evaluation::queen;
  score -= bitboard::count(opp_player.get_bitboard(Piece::Rook)) * evaluation::rook;
  return score;
}

int NaiveEngine::search(const Board& board, int current_depth) {
  if (current_depth == depth) {
    // Switch to quiescence search
    return quiescence_search(board, 0);
  }

  // TODO: This is an extremely inefficient way to check if the game has ended. Ideally chess library should provide a
  // fast method for this.
  bool game_ended = !board.generate_moves().apply_next().has_value();
  if (game_ended) {
    if (board.is_in_check())
      return evaluation::LOSING;  // Checkmate.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  MoveSet move_set = board.generate_moves();
  int board_evaluation = evaluation::LOSING;
  while (auto result = move_set.apply_next()) {
    auto& [move, new_board] = *result;
    int new_board_evaluation = search(new_board, current_depth + 1);
    board_evaluation = std::max(board_evaluation, -new_board_evaluation);
  }
  return board_evaluation;
}

int NaiveEngine::quiescence_search(const Board& board, int current_depth) {
  // TODO: This is an extremely inefficient way to check if the game has ended. Ideally chess library should provide a
  // fast method for this.
  bool game_ended = !board.generate_moves().apply_next().has_value();
  if (game_ended) {
    if (board.is_in_check())
      return evaluation::LOSING;  // Checkmate.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  if (current_depth == QUIESCENCE_SEARCH_DEPTH) return evaluate_board(board);

  MoveSet move_set = board.generate_moves();
  int board_evaluation = evaluate_board(board);
  int num_pieces = bitboard::count(board.cur_player().occupied() | board.opp_player().occupied());
  while (auto result = move_set.apply_next()) {
    auto& [move, new_board] = *result;
    int new_num_pieces = bitboard::count(new_board.cur_player().occupied() | new_board.opp_player().occupied());
    if (num_pieces == new_num_pieces) continue;  // Not a capture, ignore it.
    int new_board_evaluation = quiescence_search(new_board, current_depth + 1);
    board_evaluation = std::max(board_evaluation, -new_board_evaluation);
  }

  return board_evaluation;
}
