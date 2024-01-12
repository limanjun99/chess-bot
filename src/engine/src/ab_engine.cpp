#include "ab_engine.h"

#include <limits.h>

#include <algorithm>
#include <chrono>
#include <vector>

#include "evaluation.h"

// Evaluation is in centipawns
namespace evaluation {
constexpr int LOSING = -1000000000;
constexpr int WINNING = 1000000000;
constexpr int DRAW = 0;
constexpr int piece[6] = {300, 10000, 300, 100, 900, 500};
}  // namespace evaluation

constexpr int QUIESCENCE_SEARCH_DEPTH = 4;
constexpr int QUIESCENCE_CHECK_SEARCH_DEPTH = 2;

AlphaBetaEngine::AlphaBetaEngine(int max_depth) : max_depth{max_depth} {}

Engine::MoveInfo AlphaBetaEngine::make_move(const Board& board) {
  auto time_start = std::chrono::high_resolution_clock::now();
  normal_node_count = 0;
  quiescence_node_count = 0;

  MoveContainer moves = board.generate_moves();
  Move best_move{};
  int alpha = evaluation::LOSING;
  for (size_t i = 0; i < moves.size(); i++) {
    Move& move = moves[i];
    Board new_board = board.apply_move(move);
    int new_board_evaluation = search(new_board, -evaluation::WINNING, -alpha, 0);
    if (-new_board_evaluation > alpha || alpha == evaluation::LOSING) {
      alpha = -new_board_evaluation;
      best_move = move;
    }
  }

  auto time_end = std::chrono::high_resolution_clock::now();
  int duration = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_start).count();

  return {best_move, duration, normal_node_count, quiescence_node_count};
}

int AlphaBetaEngine::evaluate_board(const Board& board) {
  int score = 0;
  const Player& cur_player = board.cur_player();
  const Player& opp_player = board.opp_player();
  for (Piece piece : {Piece::Bishop, Piece::Knight, Piece::Pawn, Piece::Queen, Piece::Rook}) {
    score += (bitboard::count(cur_player[piece]) - bitboard::count(opp_player[piece])) *
             evaluation::piece[static_cast<int>(piece)];
    u64 cur_piece_bitboard = cur_player[piece];
    BITBOARD_ITERATE(cur_piece_bitboard, bit) { score += pst::value_of(piece, bit, board.is_white_to_move()); }
    u64 opp_piece_bitboard = opp_player[piece];
    BITBOARD_ITERATE(opp_piece_bitboard, bit) { score -= pst::value_of(piece, bit, !board.is_white_to_move()); }
  }

  return score;
}

int AlphaBetaEngine::evaluate_move_priority(const Move& move, const Board& board) {
  int priority = 0;
  if (board.is_a_capture(move)) {
    // Captures are given +1000000 priority, as they should be considered first.
    Piece captured_piece = board.opp_player().piece_at(move.get_to());
    priority += 1000000;
    priority +=
        evaluation::piece[static_cast<int>(captured_piece)] - evaluation::piece[static_cast<int>(move.get_piece())];
  }
  if (move.is_promotion()) {
    priority += evaluation::piece[static_cast<int>(move.get_promotion_piece())] -
                evaluation::piece[static_cast<int>(Piece::Pawn)];
    priority -= pst::value_of(move.get_piece(), move.get_from(), board.is_white_to_move());
    priority += pst::value_of(move.get_promotion_piece(), move.get_to(), board.is_white_to_move());
  } else {
    priority -= pst::value_of(move.get_piece(), move.get_from(), board.is_white_to_move());
    priority += pst::value_of(move.get_piece(), move.get_to(), board.is_white_to_move());
  }
  return priority;
}

int AlphaBetaEngine::search(const Board& board, int alpha, int beta, int current_depth) {
  if (current_depth == max_depth) {
    // Switch to quiescence search
    return quiescence_search(board, alpha, beta, current_depth);
  }
  normal_node_count++;

  MoveContainer moves = board.generate_moves();
  if (moves.empty()) {
    if (board.is_in_check())
      return evaluation::LOSING + current_depth;  // Checkmate, add current_depth so that shorter mates are preferred.
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
  quiescence_node_count++;
  if (!board.has_moves()) {
    if (board.is_in_check())
      return evaluation::LOSING + current_depth;  // Checkmate.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  bool is_in_check = board.is_in_check();
  if (!is_in_check && current_depth >= QUIESCENCE_SEARCH_DEPTH + max_depth) return evaluate_board(board);

  if (!is_in_check) {
    int board_evaluation = evaluate_board(board);
    if (board_evaluation >= beta) return beta;
    alpha = std::max(alpha, board_evaluation);
  }

  auto generate_moves = [&]() {
    if (is_in_check) return board.generate_moves();
    if (current_depth <= max_depth + QUIESCENCE_CHECK_SEARCH_DEPTH) return board.generate_quiescence_moves_and_checks();
    return board.generate_quiescence_moves();
  };
  MoveContainer moves = generate_moves();

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
