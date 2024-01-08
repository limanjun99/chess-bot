#include "ab_engine.h"

#include <limits.h>

#include <algorithm>
#include <vector>

#include "chess/moveset.h"

namespace evaluation {
constexpr int LOSING = -1000000000;
constexpr int WINNING = 1000000000;
constexpr int DRAW = 0;
constexpr int piece[6] = {3, 100, 3, 1, 9, 5};
constexpr int bishop = 3;
constexpr int knight = 3;
constexpr int pawn = 1;
constexpr int queen = 9;
constexpr int rook = 5;
}  // namespace evaluation

constexpr int QUIESCENCE_SEARCH_DEPTH = 5;

AlphaBetaEngine::AlphaBetaEngine(int depth) : depth{depth} {}

Move AlphaBetaEngine::make_move(const Board& board) {
  MoveSet move_set = board.generate_moves();
  std::optional<Move> best_move = std::nullopt;
  int alpha = evaluation::LOSING;
  while (auto result = move_set.apply_next()) {
    auto& [move, new_board] = *result;
    int new_board_evaluation = search(new_board, -evaluation::WINNING, -alpha, 0);
    if (-new_board_evaluation > alpha || alpha == evaluation::LOSING) {
      alpha = -new_board_evaluation;
      best_move = std::optional(move);
    }
  }
  return *best_move;
}

int AlphaBetaEngine::evaluate_board(const Board& board) {
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

int AlphaBetaEngine::search(const Board& board, int alpha, int beta, int current_depth) {
  if (current_depth == depth) {
    // Switch to quiescence search
    return quiescence_search(board, alpha, beta, current_depth);
  }

  MoveSet move_set = board.generate_moves();
  auto result = move_set.apply_next();
  bool game_ended = !result.has_value();
  if (game_ended) {
    if (board.is_in_check())
      return evaluation::LOSING + current_depth;  // Checkmate.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  std::vector<std::pair<int, std::pair<Move, Board>>> move_results;
  u64 opp_occupied = board.opp_player().occupied();
  while (result.has_value()) {
    auto& [move, new_board] = *result;
    if (opp_occupied & move.get_to()) {
      // Is capture.
      Piece capturer = board.cur_player().piece_at(move.get_from());
      Piece capturee = board.opp_player().piece_at(move.get_to());
      int move_score = evaluation::piece[static_cast<int>(capturee)] - evaluation::piece[static_cast<int>(capturer)];
      move_results.emplace_back(move_score, *result);
    } else if (move.is_promotion() || new_board.is_in_check()) {
      // Is check / promotion.
      int move_score = INT_MIN;
      move_results.emplace_back(move_score, *result);
    } else {
      move_results.emplace_back(INT_MIN, *result);
    }
    result = move_set.apply_next();
  }
  std::sort(move_results.begin(), move_results.end(), [](auto& a, auto& b) { return a.first > b.first; });

  for (auto& [_, result] : move_results) {
    auto& [move, new_board] = result;
    int new_board_evaluation = -search(new_board, -beta, -alpha, current_depth + 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }
  return alpha;
}

int AlphaBetaEngine::quiescence_search(const Board& board, int alpha, int beta, int current_depth) {
  MoveSet move_set = board.generate_moves();
  auto result = move_set.apply_next();
  bool game_ended = !result.has_value();
  if (game_ended) {
    if (board.is_in_check())
      return evaluation::LOSING + current_depth;  // Checkmate.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  if (current_depth == QUIESCENCE_SEARCH_DEPTH + depth) return evaluate_board(board);

  int board_evaluation = evaluate_board(board);
  if (board_evaluation >= beta) return beta;
  alpha = std::max(alpha, board_evaluation);
  u64 opp_occupied = board.opp_player().occupied();
  std::vector<std::pair<int, std::pair<Move, Board>>> move_results;
  while (result.has_value()) {
    auto& [move, new_board] = *result;
    if (opp_occupied & move.get_to()) {
      // Is capture.
      Piece capturer = board.cur_player().piece_at(move.get_from());
      Piece capturee = board.opp_player().piece_at(move.get_to());
      int move_score = evaluation::piece[static_cast<int>(capturee)] - evaluation::piece[static_cast<int>(capturer)];
      move_results.emplace_back(move_score, *result);
    } else if (move.is_promotion() || new_board.is_in_check()) {
      // Is check / promotion.
      int move_score = INT_MIN;
      move_results.emplace_back(move_score, *result);
    }
    result = move_set.apply_next();
  }
  std::sort(move_results.begin(), move_results.end(), [](auto& a, auto& b) { return a.first > b.first; });
  for (auto& [_, result] : move_results) {
    auto& [move, new_board] = result;
    int new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, current_depth + 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}
