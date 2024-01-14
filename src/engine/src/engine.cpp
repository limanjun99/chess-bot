#include "engine.h"

#include <limits.h>

#include <algorithm>
#include <chrono>
#include <vector>

#include "config.h"
#include "evaluation.h"

Engine::Engine() {}

Engine::MoveInfo Engine::choose_move(const Board& board, std::chrono::milliseconds search_time) {
  auto start_time = std::chrono::high_resolution_clock::now();
  normal_node_count = 0;
  quiescence_node_count = 0;
  int search_depth = 0;
  MoveContainer moves = board.generate_moves();
  Move chosen_move{};
  auto is_out_of_time = [&]() {
    auto current_time = std::chrono::high_resolution_clock::now();
    auto current_time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return current_time_spent >= search_time * config::search_time_lowerbound;
  };

  // Iterative deepening.
  while (true) {
    int alpha = evaluation::LOSING;
    Move best_move{};
    bool finished_evaluation = true;
    for (size_t i = 0; i < moves.size(); i++) {
      Board new_board = board.apply_move(moves[i]);
      int new_board_evaluation = search(new_board, -evaluation::WINNING, -alpha, search_depth);
      if (-new_board_evaluation > alpha || alpha == evaluation::LOSING) {
        alpha = -new_board_evaluation;
        best_move = moves[i];
      }
      if (is_out_of_time()) {
        finished_evaluation = false;
        break;
      }
    }

    if (finished_evaluation) chosen_move = best_move;
    if (is_out_of_time()) break;
    search_depth++;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  return {chosen_move, time_spent, search_depth, normal_node_count, quiescence_node_count};
}

int Engine::evaluate_board(const Board& board) {
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

int Engine::evaluate_move_priority(const Move& move, const Board& board) {
  int priority = 0;
  if (board.is_a_capture(move)) {
    // Captures are given +1000000 priority, as they should be considered first.
    Piece captured_piece = board.opp_player().piece_at(move.get_to());
    if (captured_piece == Piece::None) captured_piece = Piece::Pawn;  // En-passant.
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

int Engine::search(const Board& board, int alpha, int beta, int depth_left) {
  if (depth_left <= 0) {
    // Switch to quiescence search
    return quiescence_search(board, alpha, beta, 0);
  }
  normal_node_count++;

  MoveContainer moves = board.generate_moves();
  bool is_in_check = board.is_in_check();
  if (moves.empty()) {
    if (is_in_check)
      return evaluation::LOSING - depth_left;  // Checkmate, minus depth_left so that shorter mates are preferred.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  // Null move heuristic (https://www.chessprogramming.org/Null_Move_Pruning).
  if (!is_in_check && depth_left >= config::null_move_heuristic_R + 1 && beta < evaluation::WINNING) {
    Board new_board = board.skip_turn();
    int null_move_evaluation = -search(new_board, -beta, -alpha, depth_left - 1 - config::null_move_heuristic_R);
    if (null_move_evaluation >= beta) return beta;
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
    int new_board_evaluation = -search(new_board, -beta, -alpha, depth_left - 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}

int Engine::quiescence_search(const Board& board, int alpha, int beta, int depth_left) {
  quiescence_node_count++;
  if (!board.has_moves()) {
    if (board.is_in_check())
      return evaluation::LOSING - depth_left;  // Checkmate, minus depth_left so that shorter mates are preferred.
    else
      return evaluation::DRAW;  // Stalemate.
  }

  bool is_in_check = board.is_in_check();
  if (!is_in_check && depth_left <= -config::quiescence_search_depth) return evaluate_board(board);

  if (!is_in_check) {
    int board_evaluation = evaluate_board(board);
    if (board_evaluation >= beta) return beta;
    alpha = std::max(alpha, board_evaluation);
  }

  auto generate_moves = [&]() {
    if (is_in_check) return board.generate_moves();
    if (depth_left > -config::quiescence_search_check_depth) return board.generate_quiescence_moves_and_checks();
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
    int new_board_evaluation = -quiescence_search(new_board, -beta, -alpha, depth_left - 1);
    if (new_board_evaluation >= beta) return beta;
    alpha = std::max(alpha, new_board_evaluation);
  }

  return alpha;
}
