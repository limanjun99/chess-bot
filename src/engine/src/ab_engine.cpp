#include "ab_engine.h"

#include <limits.h>

#include <algorithm>
#include <vector>

#include "chess/moveset.h"

// Evaluation is in centipawns
namespace evaluation {
constexpr int LOSING = -1000000000;
constexpr int WINNING = 1000000000;
constexpr int DRAW = 0;
constexpr int piece[6] = {300, 10000, 300, 100, 900, 500};
constexpr int bishop = 3;
constexpr int knight = 3;
constexpr int pawn = 1;
constexpr int queen = 9;
constexpr int rook = 5;
}  // namespace evaluation

constexpr int QUIESCENCE_SEARCH_DEPTH = 5;
constexpr int QUIESCENCE_CHECK_SEARCH_DEPTH = 4;

AlphaBetaEngine::AlphaBetaEngine(int depth) : depth{depth} {}

Move AlphaBetaEngine::make_move(const Board& board) {
  MoveSet move_set = board.generate_moveset();
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
  score += bitboard::count(cur_player[Piece::Bishop]) * evaluation::bishop;
  score += bitboard::count(cur_player[Piece::Knight]) * evaluation::knight;
  score += bitboard::count(cur_player[Piece::Pawn]) * evaluation::pawn;
  score += bitboard::count(cur_player[Piece::Queen]) * evaluation::queen;
  score += bitboard::count(cur_player[Piece::Rook]) * evaluation::rook;
  score -= bitboard::count(opp_player[Piece::Bishop]) * evaluation::bishop;
  score -= bitboard::count(opp_player[Piece::Knight]) * evaluation::knight;
  score -= bitboard::count(opp_player[Piece::Pawn]) * evaluation::pawn;
  score -= bitboard::count(opp_player[Piece::Queen]) * evaluation::queen;
  score -= bitboard::count(opp_player[Piece::Rook]) * evaluation::rook;
  return score;
}

int AlphaBetaEngine::evaluate_move_priority(const Move& move, const Board& board) {
  int priority = 0;
  if (board.opp_player().occupied() & move.get_to()) {
    // Captures are given +1000000 priority, as they should be considered first.
    Piece captured_piece = board.opp_player().piece_at(move.get_to());
    priority += 1000000;
    priority +=
        evaluation::piece[static_cast<int>(captured_piece)] - evaluation::piece[static_cast<int>(move.get_piece())];
  }
  if (move.is_promotion()) {
    priority += evaluation::piece[static_cast<int>(move.get_promotion_piece())] -
                evaluation::piece[static_cast<int>(Piece::Pawn)];
  }
  return priority;
}

int AlphaBetaEngine::search(const Board& board, int alpha, int beta, int current_depth) {
  if (current_depth == depth) {
    // Switch to quiescence search
    return quiescence_search(board, alpha, beta, current_depth);
  }

  std::vector<Move> moves = board.generate_moves();
  if (moves.empty()) {
    if (board.is_in_check())
      return evaluation::LOSING + current_depth;  // Checkmate.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (size_t i = 0; i < moves.size(); i++) {
    move_priorities[i] = evaluate_move_priority(moves[i], board);
  }

  for (size_t i = 0; i < moves.size(); i++) {
    int best_priority = INT_MIN;
    size_t best_index = i;
    for (size_t j = i; j < moves.size(); j++) {
      if (move_priorities[j] > best_priority) {
        best_priority = move_priorities[j];
        best_index = j;
      }
    }
    std::swap(moves[i], moves[best_index]);
    std::swap(move_priorities[i], move_priorities[best_index]);
    Board new_board = board.apply_move(moves[i]);
    int new_board_evaluation = -search(new_board, -beta, -alpha, current_depth + 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}

int AlphaBetaEngine::quiescence_search(const Board& board, int alpha, int beta, int current_depth) {
  // TODO: Surely there is a faster way to check if the game is over than generating a moveset.
  MoveSet moveset = board.generate_moveset();
  if (!moveset.apply_next().has_value()) {
    if (board.is_in_check())
      return evaluation::LOSING + current_depth;  // Checkmate.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  if (current_depth == QUIESCENCE_SEARCH_DEPTH + depth) return evaluate_board(board);

  int board_evaluation = evaluate_board(board);
  if (board_evaluation >= beta) return beta;
  alpha = std::max(alpha, board_evaluation);

  std::vector<Move> moves;
  if (current_depth <= depth + QUIESCENCE_CHECK_SEARCH_DEPTH) {
    // Only generate checks and evasions below a certain depth, to prevent search explosion.
    moves = board.generate_quiescence_moves_and_checks();
  } else {
    moves = board.generate_quiescence_moves();
  }

  std::vector<int> move_priorities;
  move_priorities.reserve(moves.size());
  for (size_t i = 0; i < moves.size(); i++) {
    move_priorities[i] = evaluate_move_priority(moves[i], board);
  }

  for (size_t i = 0; i < moves.size(); i++) {
    int best_priority = INT_MIN;
    size_t best_index = i;
    for (size_t j = i; j < moves.size(); j++) {
      if (move_priorities[j] > best_priority) {
        best_priority = move_priorities[j];
        best_index = j;
      }
    }
    std::swap(moves[i], moves[best_index]);
    std::swap(move_priorities[i], move_priorities[best_index]);
    Board new_board = board.apply_move(moves[i]);
    int new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, current_depth + 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}
